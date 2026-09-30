#pragma once

#include "display_mode.h"
#include "geometry.h"

namespace fotoframe {

enum class Flip { None, Horizontal, Vertical };

// 画像を正しい向きにするための反転と回転（時計回り、度）。反転してから回転する。
struct Orientation {
    int angle;
    Flip flip;
    bool operator==(const Orientation&) const = default;
};

// EXIF の Orientation（1〜8）を反転と回転に変換する。範囲外の値は回転なしとして扱う。
Orientation orientation_from_exif(int exif_orientation);

// 90° または 270° 回転して、縦横が入れ替わって見えるか。
bool is_quarter_turn(Orientation orientation);

// 画像を画面の中央に置くときの描画先の矩形を返す。
// 矩形は回転前の向きで表し、回転は矩形の中心を軸に行う（SDL_RenderTextureRotated と同じ）。
Rect place(Size image, Orientation orientation, DisplayMode mode, Size screen);

}  // namespace fotoframe
