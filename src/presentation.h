#pragma once

#include <random>
#include <vector>

#include "display_mode.h"
#include "transition.h"

namespace fotoframe {

// 設定で指定する表示モード。Random は写真ごとに Fit と Fill から選ぶ。
enum class ModeSetting { Fit, Fill, Random };

// 1枚の写真をどう見せるか。
struct Presentation {
    DisplayMode mode;
    Effect effect;  // この写真に切り替えるときの効果
};

// 設定に従って、写真ごとの表示モードと切り替え効果を選ぶ。
class PresentationChooser {
public:
    PresentationChooser(ModeSetting mode, std::vector<Effect> effects, unsigned seed);

    Presentation next();

private:
    ModeSetting mode_;
    std::vector<Effect> effects_;
    std::mt19937 rng_;
};

}  // namespace fotoframe
