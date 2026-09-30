#include "slideshow.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

constexpr int kNoon = 12 * 60;

TEST(SlideshowTest, WaitsUntilFirstPhotoIsReady) {
    Slideshow show(60, 2, std::nullopt);

    const SlideshowFrame frame = show.update(0, kNoon, false);

    EXPECT_EQ(frame.phase, Phase::Waiting);
    EXPECT_FALSE(frame.start_transition);
}

TEST(SlideshowTest, FirstPhotoStartsTransition) {
    Slideshow show(60, 2, std::nullopt);
    show.update(0, kNoon, false);

    const SlideshowFrame frame = show.update(1, kNoon, true);

    EXPECT_EQ(frame.phase, Phase::Transitioning);
    EXPECT_TRUE(frame.start_transition);
    EXPECT_EQ(frame.progress, 0);
}

TEST(SlideshowTest, TransitionProgressesWithTime) {
    Slideshow show(60, 2, std::nullopt);
    show.update(10, kNoon, true);

    const SlideshowFrame frame = show.update(11, kNoon, false);

    EXPECT_EQ(frame.phase, Phase::Transitioning);
    EXPECT_FALSE(frame.start_transition);
    EXPECT_FLOAT_EQ(frame.progress, 0.5f);
}

TEST(SlideshowTest, TransitionFinishesAndShowsPhoto) {
    Slideshow show(60, 2, std::nullopt);
    show.update(10, kNoon, true);

    const SlideshowFrame frame = show.update(12, kNoon, false);

    EXPECT_EQ(frame.phase, Phase::Showing);
    EXPECT_TRUE(frame.finish_transition);
}

// 最初の写真の切り替えを終え、時刻 12 から表示している状態にする。
Slideshow showing_since_12() {
    Slideshow show(60, 2, std::nullopt);
    show.update(10, kNoon, true);
    show.update(12, kNoon, false);
    return show;
}

TEST(SlideshowTest, KeepsShowingUntilInterval) {
    Slideshow show = showing_since_12();

    const SlideshowFrame frame = show.update(71, kNoon, true);

    EXPECT_EQ(frame.phase, Phase::Showing);
    EXPECT_FALSE(frame.start_transition);
}

TEST(SlideshowTest, StartsTransitionAfterInterval) {
    Slideshow show = showing_since_12();

    const SlideshowFrame frame = show.update(72, kNoon, true);

    EXPECT_EQ(frame.phase, Phase::Transitioning);
    EXPECT_TRUE(frame.start_transition);
}

TEST(SlideshowTest, KeepsShowingUntilNextPhotoIsReady) {
    Slideshow show = showing_since_12();

    EXPECT_EQ(show.update(80, kNoon, false).phase, Phase::Showing);
    EXPECT_TRUE(show.update(81, kNoon, true).start_transition);
}

constexpr SleepHours kNight{23 * 60, 7 * 60};
constexpr int k2300 = 23 * 60;
constexpr int k0300 = 3 * 60;
constexpr int k0700 = 7 * 60;

// 最初の写真の切り替えを終え、時刻 12 から表示している状態にする（消灯時間帯あり）。
Slideshow showing_with_night() {
    Slideshow show(60, 2, kNight);
    show.update(10, kNoon, true);
    show.update(12, kNoon, false);
    return show;
}

TEST(SlideshowTest, EntersSleepAtSleepTime) {
    Slideshow show = showing_with_night();

    const SlideshowFrame frame = show.update(20, k2300, true);

    EXPECT_EQ(frame.phase, Phase::Sleeping);
    EXPECT_TRUE(frame.enter_sleep);
}

TEST(SlideshowTest, StaysAsleepWithoutEvents) {
    Slideshow show = showing_with_night();
    show.update(20, k2300, true);

    const SlideshowFrame frame = show.update(1000, k0300, true);

    EXPECT_EQ(frame.phase, Phase::Sleeping);
    EXPECT_FALSE(frame.enter_sleep);
    EXPECT_FALSE(frame.start_transition);
}

TEST(SlideshowTest, LeavesSleepAndRestartsInterval) {
    Slideshow show = showing_with_night();
    show.update(20, k2300, true);

    const SlideshowFrame frame = show.update(1000, k0700, true);

    EXPECT_EQ(frame.phase, Phase::Showing);
    EXPECT_TRUE(frame.leave_sleep);
    EXPECT_FALSE(show.update(1059, k0700, true).start_transition);
    EXPECT_TRUE(show.update(1060, k0700, true).start_transition);
}

TEST(SlideshowTest, SleepDuringTransitionFinishesIt) {
    Slideshow show = showing_with_night();
    show.update(72, kNoon, true);  // 切り替え開始

    const SlideshowFrame frame = show.update(73, k2300, false);

    EXPECT_EQ(frame.phase, Phase::Sleeping);
    EXPECT_TRUE(frame.enter_sleep);
    EXPECT_TRUE(frame.finish_transition);
}

TEST(SlideshowTest, LeavingSleepWithoutPhotoGoesBackToWaiting) {
    Slideshow show(60, 2, kNight);
    show.update(0, k2300, false);

    EXPECT_EQ(show.update(10, k0700, false).phase, Phase::Waiting);
}

}  // namespace
}  // namespace fotoframe
