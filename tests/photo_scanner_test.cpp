#include "photo_scanner.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>

namespace fotoframe {
namespace {

namespace fs = std::filesystem;

// テストごとに空の一時フォルダを用意し、終わったら消す。
class PhotoScannerTest : public testing::Test {
protected:
    void SetUp() override {
        root_ = fs::temp_directory_path() /
                ("fotoframe_test_" + std::string(testing::UnitTest::GetInstance()->current_test_info()->name()));
        fs::remove_all(root_);
        fs::create_directories(root_);
    }
    void TearDown() override { fs::remove_all(root_); }

    fs::path touch(const fs::path& relative) {
        const fs::path path = root_ / relative;
        fs::create_directories(path.parent_path());
        std::ofstream(path) << "x";
        return path;
    }

    static std::vector<fs::path> sorted(std::vector<fs::path> paths) {
        std::ranges::sort(paths);
        return paths;
    }

    fs::path root_;
};

TEST_F(PhotoScannerTest, FindsPhotoInRoot) {
    const fs::path photo = touch("a.jpg");

    const auto result = scan_photos(root_);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::vector<fs::path>{photo});
}

TEST_F(PhotoScannerTest, FindsPhotosInNestedSubfolders) {
    const fs::path a = touch("a.jpg");
    const fs::path b = touch("2021/b.jpg");
    const fs::path c = touch("2021/05/c.JPG");

    const auto result = scan_photos(root_);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(sorted(*result), sorted({a, b, c}));
}

TEST_F(PhotoScannerTest, IgnoresNonPhotoFiles) {
    const fs::path photo = touch("a.jpg");
    touch("movie.mp4");
    touch("sub/anim.gif");

    const auto result = scan_photos(root_);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::vector<fs::path>{photo});
}

TEST_F(PhotoScannerTest, EmptyFolderGivesEmptyList) {
    fs::create_directories(root_ / "empty_sub");

    const auto result = scan_photos(root_);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(PhotoScannerTest, MissingRootIsError) {
    EXPECT_FALSE(scan_photos(root_ / "no_such_dir").has_value());
}

}  // namespace
}  // namespace fotoframe
