#include "photo_buffer.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace fotoframe {
namespace {

// 指定したサイズのダミーの JPEG データ。
std::vector<unsigned char> bytes(size_t size) { return std::vector<unsigned char>(size, 0); }

TEST(PhotoBufferTest, EmptyBufferGivesNothing) {
    PhotoBuffer buffer(100, 1);
    EXPECT_FALSE(buffer.take().has_value());
}

TEST(PhotoBufferTest, TakeReturnsAddedPhoto) {
    PhotoBuffer buffer(100, 1);
    buffer.add("a.jpg", bytes(10));

    const auto photo = buffer.take();

    ASSERT_TRUE(photo.has_value());
    EXPECT_EQ(photo->path, "a.jpg");
    EXPECT_EQ(photo->data->size(), 10u);
}

TEST(PhotoBufferTest, UnshownPhotosComeInAddedOrder) {
    PhotoBuffer buffer(100, 1);
    buffer.add("a.jpg", bytes(10));
    buffer.add("b.jpg", bytes(10));
    buffer.add("c.jpg", bytes(10));

    EXPECT_EQ(buffer.take()->path, "a.jpg");
    EXPECT_EQ(buffer.take()->path, "b.jpg");
    EXPECT_EQ(buffer.take()->path, "c.jpg");
}

TEST(PhotoBufferTest, WhenAllShownCyclesThroughShownPhotos) {
    PhotoBuffer buffer(100, 1);
    buffer.add("a.jpg", bytes(10));
    buffer.add("b.jpg", bytes(10));
    buffer.add("c.jpg", bytes(10));
    for (int i = 0; i < 3; ++i) buffer.take();

    std::vector<std::filesystem::path> round;
    for (int i = 0; i < 3; ++i) round.push_back(buffer.take()->path);

    std::ranges::sort(round);
    EXPECT_EQ(round, (std::vector<std::filesystem::path>{"a.jpg", "b.jpg", "c.jpg"}));
}

TEST(PhotoBufferTest, LastUnshownPhotoIsNotRepeatedWhenCyclingStarts) {
    for (unsigned seed = 0; seed < 20; ++seed) {
        PhotoBuffer buffer(100, seed);
        buffer.add("a.jpg", bytes(10));
        buffer.add("b.jpg", bytes(10));
        buffer.take();
        buffer.take();

        EXPECT_EQ(buffer.take()->path, "a.jpg") << "seed " << seed;
    }
}

TEST(PhotoBufferTest, AddingOverCapacityDiscardsOldestShownPhoto) {
    PhotoBuffer buffer(30, 1);
    buffer.add("a.jpg", bytes(10));
    buffer.add("b.jpg", bytes(10));
    buffer.add("c.jpg", bytes(10));
    buffer.take();  // a
    buffer.take();  // b

    EXPECT_EQ(buffer.add("d.jpg", bytes(10)), PhotoBuffer::AddResult::Added);

    // a が捨てられ、b は残っている
    EXPECT_EQ(buffer.take()->path, "c.jpg");
    EXPECT_EQ(buffer.take()->path, "d.jpg");
    std::vector<std::filesystem::path> shown;
    for (int i = 0; i < 6; ++i) shown.push_back(buffer.take()->path);
    EXPECT_EQ(std::ranges::count(shown, "a.jpg"), 0);
    EXPECT_GT(std::ranges::count(shown, "b.jpg"), 0);
}

TEST(PhotoBufferTest, UnshownPhotosAreNeverDiscarded) {
    PhotoBuffer buffer(30, 1);
    buffer.add("a.jpg", bytes(10));
    buffer.add("b.jpg", bytes(10));
    buffer.add("c.jpg", bytes(10));
    buffer.take();  // a

    EXPECT_EQ(buffer.add("d.jpg", bytes(20)), PhotoBuffer::AddResult::Full);

    EXPECT_EQ(buffer.take()->path, "b.jpg");
    EXPECT_EQ(buffer.take()->path, "c.jpg");
}

TEST(PhotoBufferTest, PhotoLargerThanCapacityIsRejected) {
    PhotoBuffer buffer(30, 1);

    EXPECT_EQ(buffer.add("big.jpg", bytes(31)), PhotoBuffer::AddResult::TooLarge);

    EXPECT_FALSE(buffer.take().has_value());
}

TEST(PhotoBufferTest, AddingShownPhotoAgainMakesItUnshownWithoutDuplicate) {
    PhotoBuffer buffer(20, 1);
    buffer.add("a.jpg", bytes(10));
    buffer.add("b.jpg", bytes(10));
    buffer.take();  // a

    EXPECT_EQ(buffer.add("a.jpg", bytes(10)), PhotoBuffer::AddResult::Added);

    EXPECT_EQ(buffer.used_bytes(), 20u);
    EXPECT_EQ(buffer.take()->path, "b.jpg");
    EXPECT_EQ(buffer.take()->path, "a.jpg");
}

TEST(PhotoBufferTest, AddingUnshownPhotoAgainIsReported) {
    PhotoBuffer buffer(100, 1);
    buffer.add("a.jpg", bytes(10));

    EXPECT_EQ(buffer.add("a.jpg", bytes(10)), PhotoBuffer::AddResult::AlreadyQueued);
    EXPECT_EQ(buffer.used_bytes(), 10u);

    buffer.take();
    // 表示済みの a だけが残っていて、次も a が出る
    EXPECT_EQ(buffer.take()->path, "a.jpg");
}

}  // namespace
}  // namespace fotoframe
