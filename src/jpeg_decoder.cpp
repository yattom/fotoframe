#include "jpeg_decoder.h"

#include <libexif/exif-data.h>
#include <turbojpeg.h>

#include "decode_scale.h"

namespace fotoframe {

namespace {

int read_exif_orientation(const std::vector<unsigned char>& jpeg) {
    ExifData* exif = exif_data_new_from_data(jpeg.data(), jpeg.size());
    if (!exif) return 1;
    int orientation = 1;
    if (ExifEntry* entry = exif_data_get_entry(exif, EXIF_TAG_ORIENTATION)) {
        orientation = exif_get_short(entry->data, exif_data_get_byte_order(exif));
    }
    exif_data_unref(exif);
    return orientation;
}

std::vector<Scale> scaling_factors() {
    int count = 0;
    const tjscalingfactor* factors = tjGetScalingFactors(&count);
    std::vector<Scale> scales;
    for (int i = 0; i < count; ++i) scales.push_back({factors[i].num, factors[i].denom});
    return scales;
}

}  // namespace

JpegDecoder::JpegDecoder() : handle_(tjInitDecompress()) {}

JpegDecoder::~JpegDecoder() { tjDestroy(handle_); }

std::optional<DecodedImage> JpegDecoder::decode(const std::vector<unsigned char>& jpeg, DisplayMode mode,
                                                Size screen) {
    if (!handle_) return std::nullopt;
    int width = 0, height = 0, subsampling = 0, colorspace = 0;
    if (tjDecompressHeader3(handle_, jpeg.data(), jpeg.size(), &width, &height, &subsampling, &colorspace) != 0) {
        return std::nullopt;
    }

    const Orientation orientation = orientation_from_exif(read_exif_orientation(jpeg));
    static const std::vector<Scale> factors = scaling_factors();
    const Scale scale = choose_decode_scale({width, height}, is_quarter_turn(orientation), mode, screen, factors);
    const tjscalingfactor tj_scale{scale.num, scale.denom};

    DecodedImage image{{TJSCALED(width, tj_scale), TJSCALED(height, tj_scale)}, {}, orientation};
    image.pixels.resize(static_cast<size_t>(image.size.width) * image.size.height * 4);
    // 警告（データの末尾が欠けているなど）は表示できるので許容し、致命的なエラーだけ失敗にする。
    if (tjDecompress2(handle_, jpeg.data(), jpeg.size(), image.pixels.data(), image.size.width, 0, image.size.height,
                      TJPF_RGBX, 0) != 0 &&
        tjGetErrorCode(handle_) == TJERR_FATAL) {
        return std::nullopt;
    }
    return image;
}

}  // namespace fotoframe
