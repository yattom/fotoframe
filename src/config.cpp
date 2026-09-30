#include "config.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <toml++/toml.hpp>

namespace fotoframe {

namespace {

// 設定の値が不正なときに投げる。parse_config の中だけで使う。
struct ConfigError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

double read_number(const toml::table& table, const char* key, double fallback) {
    const toml::node* node = table.get(key);
    if (!node) return fallback;
    const auto value = node->value<double>();
    if (!value) throw ConfigError(std::string(key) + " must be a number");
    return *value;
}

double read_positive(const toml::table& table, const char* key, double fallback) {
    const double value = read_number(table, key, fallback);
    if (value <= 0) throw ConfigError(std::string(key) + " must be greater than 0");
    return value;
}

std::string read_string(const toml::table& table, const char* key, const std::string& fallback) {
    const toml::node* node = table.get(key);
    if (!node) return fallback;
    const auto value = node->value<std::string>();
    if (!value) throw ConfigError(std::string(key) + " must be a string");
    return *value;
}

ModeSetting parse_mode(const std::string& text) {
    if (text == "fit") return ModeSetting::Fit;
    if (text == "fill") return ModeSetting::Fill;
    if (text == "random") return ModeSetting::Random;
    throw ConfigError("display_mode must be \"fit\", \"fill\" or \"random\": " + text);
}

Effect parse_effect(const std::string& text) {
    if (text == "none") return Effect::None;
    if (text == "fade") return Effect::Fade;
    if (text == "slide_horizontal") return Effect::SlideHorizontal;
    if (text == "slide_vertical") return Effect::SlideVertical;
    throw ConfigError("unknown effect: " + text);
}

std::vector<Effect> read_effects(const toml::table& table, const std::vector<Effect>& fallback) {
    const toml::node* node = table.get("effects");
    if (!node) return fallback;
    const toml::array* array = node->as_array();
    if (!array) throw ConfigError("effects must be an array of strings");
    std::vector<Effect> effects;
    for (const toml::node& item : *array) {
        const auto text = item.value<std::string>();
        if (!text) throw ConfigError("effects must be an array of strings");
        effects.push_back(parse_effect(*text));
    }
    return effects;
}

// "HH:MM" を 0:00 からの分数にする。
int parse_time_of_day(const char* key, const std::string& text) {
    int hour = -1, minute = -1;
    char rest = 0;
    if (std::sscanf(text.c_str(), "%d:%d%c", &hour, &minute, &rest) != 2 || hour < 0 || hour > 23 || minute < 0 ||
        minute > 59) {
        throw ConfigError(std::string(key) + " must be \"HH:MM\": " + text);
    }
    return hour * 60 + minute;
}

std::optional<SleepHours> read_sleep_hours(const toml::table& table) {
    const std::string start = read_string(table, "sleep_start", "");
    const std::string end = read_string(table, "sleep_end", "");
    if (start.empty() && end.empty()) return std::nullopt;
    if (start.empty() || end.empty()) throw ConfigError("sleep_start and sleep_end must be set together");
    return SleepHours{parse_time_of_day("sleep_start", start), parse_time_of_day("sleep_end", end)};
}

Config read_config(const toml::table& table) {
    Config config;
    config.photo_dir = read_string(table, "photo_dir", "");
    if (config.photo_dir.empty()) throw ConfigError("photo_dir is required");

    config.interval_seconds = read_positive(table, "interval_seconds", config.interval_seconds);
    config.transition_seconds = read_number(table, "transition_seconds", config.transition_seconds);
    if (config.transition_seconds < 0 || config.transition_seconds >= config.interval_seconds) {
        throw ConfigError("transition_seconds must be 0 or more and less than interval_seconds");
    }
    config.display_mode = parse_mode(read_string(table, "display_mode", "random"));
    config.effects = read_effects(table, config.effects);
    config.rescan_minutes = read_positive(table, "rescan_minutes", config.rescan_minutes);
    config.buffer_megabytes = static_cast<int>(read_positive(table, "buffer_megabytes", config.buffer_megabytes));
    config.sleep_hours = read_sleep_hours(table);
    return config;
}

}  // namespace

ConfigResult parse_config(std::string_view toml_text) {
    try {
        return {read_config(toml::parse(toml_text)), ""};
    } catch (const toml::parse_error& e) {
        return {std::nullopt, std::string("TOML syntax error: ") + std::string(e.description())};
    } catch (const ConfigError& e) {
        return {std::nullopt, e.what()};
    }
}

ConfigResult load_config(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) return {std::nullopt, "cannot open config file: " + path.string()};
    std::stringstream text;
    text << in.rdbuf();
    return parse_config(text.str());
}

}  // namespace fotoframe
