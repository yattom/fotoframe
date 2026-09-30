#include "config.h"

#include <gtest/gtest.h>

#include <fstream>

namespace fotoframe {
namespace {

TEST(ConfigTest, OnlyPhotoDirUsesDefaults) {
    const ConfigResult result = parse_config(R"(photo_dir = "/mnt/photos")");

    ASSERT_TRUE(result.config.has_value()) << result.error;
    const Config& config = *result.config;
    EXPECT_EQ(config.photo_dir, "/mnt/photos");
    EXPECT_EQ(config.interval_seconds, 60);
    EXPECT_EQ(config.transition_seconds, 1.5);
    EXPECT_EQ(config.display_mode, ModeSetting::Random);
    EXPECT_EQ(config.effects,
              (std::vector<Effect>{Effect::None, Effect::Fade, Effect::SlideHorizontal, Effect::SlideVertical}));
    EXPECT_EQ(config.rescan_minutes, 60);
    EXPECT_EQ(config.buffer_megabytes, 300);
    EXPECT_FALSE(config.sleep_hours.has_value());
}

TEST(ConfigTest, AllItems) {
    const ConfigResult result = parse_config(R"(
        photo_dir = "/mnt/photos"
        interval_seconds = 30
        transition_seconds = 2
        display_mode = "fill"
        effects = ["fade", "slide_vertical"]
        rescan_minutes = 1440
        buffer_megabytes = 100
        sleep_start = "23:00"
        sleep_end = "07:30"
    )");

    ASSERT_TRUE(result.config.has_value()) << result.error;
    const Config& config = *result.config;
    EXPECT_EQ(config.interval_seconds, 30);
    EXPECT_EQ(config.transition_seconds, 2);
    EXPECT_EQ(config.display_mode, ModeSetting::Fill);
    EXPECT_EQ(config.effects, (std::vector<Effect>{Effect::Fade, Effect::SlideVertical}));
    EXPECT_EQ(config.rescan_minutes, 1440);
    EXPECT_EQ(config.buffer_megabytes, 100);
    ASSERT_TRUE(config.sleep_hours.has_value());
    EXPECT_EQ(config.sleep_hours->start_minutes, 23 * 60);
    EXPECT_EQ(config.sleep_hours->end_minutes, 7 * 60 + 30);
}

// 設定がエラーになり、エラーメッセージに expected が含まれることを確かめる。
void expect_error_from(const ConfigResult& result, const std::string& expected) {
    EXPECT_FALSE(result.config.has_value());
    EXPECT_NE(result.error.find(expected), std::string::npos) << result.error;
}

void expect_error(std::string_view toml_text, const std::string& expected) {
    expect_error_from(parse_config(toml_text), expected);
}

TEST(ConfigTest, PhotoDirIsRequired) { expect_error("interval_seconds = 30", "photo_dir"); }

TEST(ConfigTest, SyntaxErrorIsReported) { expect_error("photo_dir = ", "TOML syntax error"); }

TEST(ConfigTest, UnknownDisplayModeIsError) {
    expect_error(R"(photo_dir = "/p"
                    display_mode = "stretch")",
                 "display_mode");
}

TEST(ConfigTest, UnknownEffectIsError) {
    expect_error(R"(photo_dir = "/p"
                    effects = ["fade", "zoom"])",
                 "zoom");
}

TEST(ConfigTest, WrongTypeIsError) {
    expect_error(R"(photo_dir = "/p"
                    interval_seconds = "60")",
                 "interval_seconds");
}

TEST(ConfigTest, NonPositiveNumbersAreErrors) {
    expect_error(R"(photo_dir = "/p"
                    interval_seconds = 0)",
                 "interval_seconds");
    expect_error(R"(photo_dir = "/p"
                    transition_seconds = -1)",
                 "transition_seconds");
    expect_error(R"(photo_dir = "/p"
                    rescan_minutes = 0)",
                 "rescan_minutes");
    expect_error(R"(photo_dir = "/p"
                    buffer_megabytes = 0)",
                 "buffer_megabytes");
}

TEST(ConfigTest, TransitionMustBeShorterThanInterval) {
    expect_error(R"(photo_dir = "/p"
                    interval_seconds = 2
                    transition_seconds = 2)",
                 "transition_seconds");
}

TEST(ConfigTest, InvalidSleepTimeIsError) {
    expect_error(R"(photo_dir = "/p"
                    sleep_start = "24:00"
                    sleep_end = "07:00")",
                 "sleep_start");
    expect_error(R"(photo_dir = "/p"
                    sleep_start = "23:00"
                    sleep_end = "7")",
                 "sleep_end");
}

TEST(ConfigTest, SleepStartAndEndMustBeSetTogether) {
    expect_error(R"(photo_dir = "/p"
                    sleep_start = "23:00")",
                 "sleep_end");
}

TEST(ConfigTest, LoadsFromFile) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "fotoframe_config_test.toml";
    std::ofstream(path) << R"(photo_dir = "/mnt/photos")";

    const ConfigResult result = load_config(path);

    std::filesystem::remove(path);
    ASSERT_TRUE(result.config.has_value()) << result.error;
    EXPECT_EQ(result.config->photo_dir, "/mnt/photos");
}

TEST(ConfigTest, MissingFileIsError) { expect_error_from(load_config("/no/such/config.toml"), "/no/such/config.toml"); }

}  // namespace
}  // namespace fotoframe
