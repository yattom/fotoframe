#include "photo_buffer.h"

#include <algorithm>

namespace fotoframe {

PhotoBuffer::PhotoBuffer(size_t capacity_bytes, unsigned seed) : capacity_bytes_(capacity_bytes), shown_deck_(seed) {}

PhotoBuffer::AddResult PhotoBuffer::add(const std::filesystem::path& path, std::vector<unsigned char> data) {
    std::lock_guard lock(mutex_);
    const size_t size = data.size();
    if (size > capacity_bytes_) return AddResult::TooLarge;
    if (std::ranges::find(unshown_, path, &BufferedPhoto::path) != unshown_.end()) return AddResult::AlreadyQueued;

    // 表示済みの写真はすべて捨てられるので、それを含めて空きが足りるか判定する。
    size_t shown_bytes = 0;
    for (const BufferedPhoto& photo : shown_) shown_bytes += photo.data->size();
    if (capacity_bytes_ - used_bytes_ + shown_bytes < size) return AddResult::Full;

    remove_shown(path);
    while (capacity_bytes_ - used_bytes_ < size) {
        used_bytes_ -= shown_.front().data->size();
        shown_.pop_front();
    }
    unshown_.push_back({path, std::make_shared<const std::vector<unsigned char>>(std::move(data))});
    used_bytes_ += size;
    return AddResult::Added;
}

std::optional<BufferedPhoto> PhotoBuffer::take() {
    std::lock_guard lock(mutex_);
    if (unshown_.empty()) return take_shown();
    BufferedPhoto photo = unshown_.front();
    unshown_.pop_front();
    shown_.push_back(photo);
    return photo;
}

size_t PhotoBuffer::used_bytes() const {
    std::lock_guard lock(mutex_);
    return used_bytes_;
}

void PhotoBuffer::remove_shown(const std::filesystem::path& path) {
    const auto it = std::ranges::find(shown_, path, &BufferedPhoto::path);
    if (it == shown_.end()) return;
    used_bytes_ -= it->data->size();
    shown_.erase(it);
}

std::optional<BufferedPhoto> PhotoBuffer::take_shown() {
    std::vector<std::filesystem::path> paths;
    for (const BufferedPhoto& photo : shown_) paths.push_back(photo.path);
    shown_deck_.update(paths);
    auto chosen = shown_deck_.next();
    if (!chosen) return std::nullopt;
    // 直前に表示した写真が続けて出ないようにする。
    if (shown_.size() > 1 && *chosen == shown_.back().path) chosen = shown_deck_.next();

    // 選んだ写真を「表示したのが最も新しい」位置に移す。
    const auto it = std::ranges::find(shown_, *chosen, &BufferedPhoto::path);
    BufferedPhoto photo = *it;
    shown_.erase(it);
    shown_.push_back(photo);
    return photo;
}

}  // namespace fotoframe
