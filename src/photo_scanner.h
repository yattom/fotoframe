#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace fotoframe {

// root 以下のすべてのサブフォルダをたどり、写真ファイルのパスを返す。順序は不定。
// root が読めない、または走査の途中で読めなくなったときは nullopt を返す。
std::optional<std::vector<std::filesystem::path>> scan_photos(const std::filesystem::path& root);

}  // namespace fotoframe
