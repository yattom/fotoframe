#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "presentation.h"
#include "sleep_schedule.h"
#include "transition.h"

namespace fotoframe {

// 設定ファイルの内容。値は省略時の既定値。
struct Config {
    std::filesystem::path photo_dir;
    double interval_seconds = 60;      // 1枚を表示する時間
    double transition_seconds = 1.5;   // 切り替え効果にかける時間
    ModeSetting display_mode = ModeSetting::Random;
    std::vector<Effect> effects = {Effect::None, Effect::Fade, Effect::SlideHorizontal, Effect::SlideVertical};
    double rescan_minutes = 60;        // 写真フォルダを再走査する間隔
    int buffer_megabytes = 300;        // 先読みバッファの容量
    std::optional<SleepHours> sleep_hours;  // 消灯する時間帯。なければ消灯しない
};

// config と error のどちらか一方が意味を持つ。
struct ConfigResult {
    std::optional<Config> config;
    std::string error;
};

// TOML 形式の設定を読み取る。
ConfigResult parse_config(std::string_view toml_text);

// TOML 形式の設定ファイルを読み取る。
ConfigResult load_config(const std::filesystem::path& path);

}  // namespace fotoframe
