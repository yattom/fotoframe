#pragma once

#include "geometry.h"

namespace fotoframe {

// 写真を切り替えるときの効果。
enum class Effect {
    None,             // すぐに切り替える
    Fade,             // クロスフェード
    SlideHorizontal,  // 新しい写真が右から入り、古い写真を左へ押し出す
    SlideVertical,    // 新しい写真が下から入り、古い写真を上へ押し出す
};

// 1枚の写真の描画状態。dx、dy は通常の位置からのずれ（ピクセル）。
struct LayerState {
    float alpha;
    float dx;
    float dy;
    bool operator==(const LayerState&) const = default;
};

struct TransitionFrame {
    LayerState outgoing;  // 消えていく写真
    LayerState incoming;  // 現れる写真
    bool operator==(const TransitionFrame&) const = default;
};

// 切り替えの進み具合 progress（0〜1）における、2枚の写真の描画状態を返す。
TransitionFrame transition_frame(Effect effect, float progress, Size screen);

}  // namespace fotoframe
