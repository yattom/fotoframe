#include "decode_scale.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

// libturbojpeg 2.1 が返す縮小率の一覧と同じもの。
const std::vector<Scale> kFactors = {{2, 1}, {15, 8}, {7, 4}, {13, 8}, {3, 2}, {11, 8}, {5, 4}, {9, 8},
                                     {1, 1}, {7, 8},  {3, 4}, {5, 8},  {1, 2}, {3, 8},  {1, 4}, {1, 8}};

constexpr Size kScreen{1920, 1080};

TEST(DecodeScaleTest, FitNeedsOnlyOneSideToReachScreen) {
    // 3/8 で 1512x1134。高さが画面の 1080 以上なので足りる
    EXPECT_EQ(choose_decode_scale({4032, 3024}, false, DisplayMode::Fit, kScreen, kFactors), (Scale{3, 8}));
}


TEST(DecodeScaleTest, FillNeedsBothSidesToReachScreen) {
    // 3/8 では幅 1512 が足りないので 1/2（2016x1512）
    EXPECT_EQ(choose_decode_scale({4032, 3024}, false, DisplayMode::Fill, kScreen, kFactors), (Scale{1, 2}));
}

TEST(DecodeScaleTest, QuarterTurnSwapsWidthAndHeight) {
    // 縦向きに表示するので 3024x4032 として扱う。幅 1920 以上になるのは 3/4（2268）
    EXPECT_EQ(choose_decode_scale({4032, 3024}, true, DisplayMode::Fill, kScreen, kFactors), (Scale{3, 4}));
}

TEST(DecodeScaleTest, SmallImageIsNotScaled) {
    EXPECT_EQ(choose_decode_scale({800, 600}, false, DisplayMode::Fit, kScreen, kFactors), (Scale{1, 1}));
    EXPECT_EQ(choose_decode_scale({1000, 700}, false, DisplayMode::Fill, kScreen, kFactors), (Scale{1, 1}));
}

}  // namespace
}  // namespace fotoframe
