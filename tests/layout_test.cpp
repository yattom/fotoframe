#include "layout.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

TEST(LayoutTest, ExifOrientationToRotationAndFlip) {
    EXPECT_EQ(orientation_from_exif(1), (Orientation{0, Flip::None}));
    EXPECT_EQ(orientation_from_exif(2), (Orientation{0, Flip::Horizontal}));
    EXPECT_EQ(orientation_from_exif(3), (Orientation{180, Flip::None}));
    EXPECT_EQ(orientation_from_exif(4), (Orientation{0, Flip::Vertical}));
    EXPECT_EQ(orientation_from_exif(5), (Orientation{90, Flip::Vertical}));
    EXPECT_EQ(orientation_from_exif(6), (Orientation{90, Flip::None}));
    EXPECT_EQ(orientation_from_exif(7), (Orientation{90, Flip::Horizontal}));
    EXPECT_EQ(orientation_from_exif(8), (Orientation{270, Flip::None}));
}

TEST(LayoutTest, UnknownExifOrientationIsUpright) {
    EXPECT_EQ(orientation_from_exif(0), (Orientation{0, Flip::None}));
    EXPECT_EQ(orientation_from_exif(9), (Orientation{0, Flip::None}));
}

constexpr Size kScreen{1920, 1080};
constexpr Orientation kUpright{0, Flip::None};

TEST(LayoutTest, FitShowsWholeImageWithBars) {
    // 2016x1512 を高さ 1080 に合わせると 1440x1080、左右に 240 ずつ余白
    EXPECT_EQ(place({2016, 1512}, kUpright, DisplayMode::Fit, kScreen), (Rect{240, 0, 1440, 1080}));
}

TEST(LayoutTest, FillCoversScreenAndCropsOverflow) {
    // 2016x1512 を幅 1920 に合わせると 1920x1440、上下に 180 ずつはみ出る
    EXPECT_EQ(place({2016, 1512}, kUpright, DisplayMode::Fill, kScreen), (Rect{0, -180, 1920, 1440}));
}

TEST(LayoutTest, QuarterTurnFitsRotatedSize) {
    // 90° 回転すると 1512x2016 に見える。高さ 1080 に合わせると倍率 1080/2016。
    // 回転前の矩形は 1080x810 で、中心を画面中央に置く
    const Rect rect = place({2016, 1512}, Orientation{90, Flip::None}, DisplayMode::Fit, kScreen);
    EXPECT_FLOAT_EQ(rect.width, 1080);
    EXPECT_FLOAT_EQ(rect.height, 810);
    EXPECT_FLOAT_EQ(rect.x, 420);
    EXPECT_FLOAT_EQ(rect.y, 135);
}

}  // namespace
}  // namespace fotoframe
