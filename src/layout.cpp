#include "layout.h"

#include <algorithm>

namespace fotoframe {

Orientation orientation_from_exif(int exif_orientation) {
    switch (exif_orientation) {
        case 2: return {0, Flip::Horizontal};
        case 3: return {180, Flip::None};
        case 4: return {0, Flip::Vertical};
        case 5: return {90, Flip::Vertical};
        case 6: return {90, Flip::None};
        case 7: return {90, Flip::Horizontal};
        case 8: return {270, Flip::None};
        default: return {0, Flip::None};
    }
}

bool is_quarter_turn(Orientation orientation) { return orientation.angle == 90 || orientation.angle == 270; }

Rect place(Size image, Orientation orientation, DisplayMode mode, Size screen) {
    // 倍率は、回転後に画面上で見える大きさで決める。
    const bool quarter = is_quarter_turn(orientation);
    const float shown_width = quarter ? image.height : image.width;
    const float shown_height = quarter ? image.width : image.height;
    const float kx = screen.width / shown_width;
    const float ky = screen.height / shown_height;
    const float k = (mode == DisplayMode::Fit) ? std::min(kx, ky) : std::max(kx, ky);
    const float w = image.width * k;
    const float h = image.height * k;
    return {(screen.width - w) / 2, (screen.height - h) / 2, w, h};
}

}  // namespace fotoframe
