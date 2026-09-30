#include "photo_filter.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace fotoframe {

bool is_photo(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return ext == ".jpg" || ext == ".jpeg";
}

}  // namespace fotoframe
