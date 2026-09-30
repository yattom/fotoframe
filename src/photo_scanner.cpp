#include "photo_scanner.h"

#include "photo_filter.h"

namespace fotoframe {

namespace fs = std::filesystem;

std::optional<std::vector<fs::path>> scan_photos(const fs::path& root) {
    // NAS の切断などで途中から読めなくなることがあるので、例外ではなくエラーコードで判定する。
    std::error_code error;
    fs::recursive_directory_iterator it(root, error);
    if (error) return std::nullopt;

    std::vector<fs::path> photos;
    for (; it != fs::recursive_directory_iterator(); it.increment(error)) {
        if (error) return std::nullopt;
        if (it->is_regular_file(error) && is_photo(it->path())) photos.push_back(it->path());
    }
    if (error) return std::nullopt;
    return photos;
}

}  // namespace fotoframe
