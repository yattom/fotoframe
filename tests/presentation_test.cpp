#include "presentation.h"

#include <gtest/gtest.h>

#include <set>

namespace fotoframe {
namespace {

TEST(PresentationTest, FixedModeIsAlwaysUsed) {
    PresentationChooser chooser(ModeSetting::Fill, {Effect::Fade}, 1);
    for (int i = 0; i < 20; ++i) EXPECT_EQ(chooser.next().mode, DisplayMode::Fill);
}

TEST(PresentationTest, RandomModeUsesBothModes) {
    PresentationChooser chooser(ModeSetting::Random, {Effect::Fade}, 1);
    std::set<DisplayMode> modes;
    for (int i = 0; i < 50; ++i) modes.insert(chooser.next().mode);
    EXPECT_EQ(modes, (std::set<DisplayMode>{DisplayMode::Fit, DisplayMode::Fill}));
}

TEST(PresentationTest, EffectsAreChosenOnlyFromCandidates) {
    PresentationChooser chooser(ModeSetting::Fit, {Effect::Fade, Effect::SlideVertical}, 1);
    std::set<Effect> effects;
    for (int i = 0; i < 50; ++i) effects.insert(chooser.next().effect);
    EXPECT_EQ(effects, (std::set<Effect>{Effect::Fade, Effect::SlideVertical}));
}

TEST(PresentationTest, NoCandidatesMeansNoEffect) {
    PresentationChooser chooser(ModeSetting::Fit, {}, 1);
    EXPECT_EQ(chooser.next().effect, Effect::None);
}

}  // namespace
}  // namespace fotoframe
