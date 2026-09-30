#include "photo_filter.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

TEST(PhotoFilterTest, JpgIsPhoto) {
    EXPECT_TRUE(is_photo("dir/2021-01-04 13.02.31.jpg"));
}

TEST(PhotoFilterTest, ExtensionIsCaseInsensitive) {
    EXPECT_TRUE(is_photo("IMG_0001.JPG"));
    EXPECT_TRUE(is_photo("IMG_0001.Jpg"));
}

TEST(PhotoFilterTest, JpegExtensionIsPhoto) {
    EXPECT_TRUE(is_photo("photo.jpeg"));
    EXPECT_TRUE(is_photo("photo.JPEG"));
}

TEST(PhotoFilterTest, OtherFilesAreNotPhotos) {
    EXPECT_FALSE(is_photo("movie.mp4"));
    EXPECT_FALSE(is_photo("anim.gif"));
    EXPECT_FALSE(is_photo("jpg"));
    EXPECT_FALSE(is_photo("noextension"));
    EXPECT_FALSE(is_photo("photo.jpg.txt"));
}

}  // namespace
}  // namespace fotoframe
