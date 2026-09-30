#pragma once

#include <optional>
#include <vector>

#include "display_mode.h"
#include "geometry.h"
#include "layout.h"

namespace fotoframe {

// デコード済みの画像。pixels は RGBX（1ピクセル4バイト）。向きは描画時に適用する。
struct DecodedImage {
    Size size;
    std::vector<unsigned char> pixels;
    Orientation orientation;
};

// libturbojpeg と libexif で JPEG をデコードする。
class JpegDecoder {
public:
    JpegDecoder();
    ~JpegDecoder();
    JpegDecoder(const JpegDecoder&) = delete;
    JpegDecoder& operator=(const JpegDecoder&) = delete;

    // 表示モードと画面サイズに足りる範囲で縮小しながらデコードする。失敗したら nullopt。
    std::optional<DecodedImage> decode(const std::vector<unsigned char>& jpeg, DisplayMode mode, Size screen);

private:
    void* handle_;  // tjhandle
};

}  // namespace fotoframe
