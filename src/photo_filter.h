#pragma once

#include <filesystem>

namespace fotoframe {

// 表示対象の写真ファイルか判定する。
bool is_photo(const std::filesystem::path& path);

}  // namespace fotoframe
