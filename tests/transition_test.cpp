#include "transition.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

constexpr Size kScreen{1920, 1080};

TEST(TransitionTest, FadeCrossesAlpha) {
    EXPECT_EQ(transition_frame(Effect::Fade, 0.0f, kScreen), (TransitionFrame{{1, 0, 0}, {0, 0, 0}}));
    EXPECT_EQ(transition_frame(Effect::Fade, 0.25f, kScreen), (TransitionFrame{{0.75f, 0, 0}, {0.25f, 0, 0}}));
    EXPECT_EQ(transition_frame(Effect::Fade, 1.0f, kScreen), (TransitionFrame{{0, 0, 0}, {1, 0, 0}}));
}

TEST(TransitionTest, SlideHorizontalPushesOldPhotoLeft) {
    EXPECT_EQ(transition_frame(Effect::SlideHorizontal, 0.0f, kScreen), (TransitionFrame{{1, 0, 0}, {1, 1920, 0}}));
    EXPECT_EQ(transition_frame(Effect::SlideHorizontal, 0.25f, kScreen),
              (TransitionFrame{{1, -480, 0}, {1, 1440, 0}}));
    EXPECT_EQ(transition_frame(Effect::SlideHorizontal, 1.0f, kScreen), (TransitionFrame{{1, -1920, 0}, {1, 0, 0}}));
}

TEST(TransitionTest, SlideVerticalPushesOldPhotoUp) {
    EXPECT_EQ(transition_frame(Effect::SlideVertical, 0.0f, kScreen), (TransitionFrame{{1, 0, 0}, {1, 0, 1080}}));
    EXPECT_EQ(transition_frame(Effect::SlideVertical, 0.25f, kScreen), (TransitionFrame{{1, 0, -270}, {1, 0, 810}}));
    EXPECT_EQ(transition_frame(Effect::SlideVertical, 1.0f, kScreen), (TransitionFrame{{1, 0, -1080}, {1, 0, 0}}));
}

TEST(TransitionTest, NoneShowsOnlyNewPhoto) {
    EXPECT_EQ(transition_frame(Effect::None, 0.0f, kScreen), (TransitionFrame{{0, 0, 0}, {1, 0, 0}}));
}

}  // namespace
}  // namespace fotoframe
