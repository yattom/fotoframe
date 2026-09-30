#include "decode_scale.h"

#include <utility>

namespace fotoframe {

namespace {

// libturbojpeg と同じく切り上げる（TJSCALED と同じ計算）。
int scaled(int dimension, Scale scale) { return (dimension * scale.num + scale.denom - 1) / scale.denom; }

}  // namespace

Scale choose_decode_scale(Size image, bool quarter_turn, DisplayMode mode, Size screen,
                          const std::vector<Scale>& factors) {
    Scale best{1, 1};
    long best_area = static_cast<long>(image.width) * image.height;
    for (const Scale& scale : factors) {
        int w = scaled(image.width, scale);
        int h = scaled(image.height, scale);
        if (quarter_turn) std::swap(w, h);
        // fit は縦横どちらかが画面に届けば、fill は両方が届けば、拡大せずに表示できる。
        const bool enough = (mode == DisplayMode::Fit) ? (w >= screen.width || h >= screen.height)
                                                       : (w >= screen.width && h >= screen.height);
        const long area = static_cast<long>(w) * h;
        if (enough && area < best_area) {
            best = scale;
            best_area = area;
        }
    }
    return best;
}

}  // namespace fotoframe
