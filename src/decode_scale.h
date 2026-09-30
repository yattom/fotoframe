#pragma once

#include <vector>

#include "display_mode.h"
#include "geometry.h"

namespace fotoframe {

// デコード時の縮小率（num / denom）。
struct Scale {
    int num;
    int denom;
    bool operator==(const Scale&) const = default;
};

// 表示に足りる範囲で、デコード後の画像が最も小さくなる縮小率を factors から選ぶ。
// quarter_turn は、EXIF の向きにより 90° または 270° 回転して表示すること。
Scale choose_decode_scale(Size image, bool quarter_turn, DisplayMode mode, Size screen,
                          const std::vector<Scale>& factors);

}  // namespace fotoframe
