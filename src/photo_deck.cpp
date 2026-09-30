#include "photo_deck.h"

#include <algorithm>
#include <set>

namespace fotoframe {

namespace fs = std::filesystem;

PhotoDeck::PhotoDeck(unsigned seed) : rng_(seed) {}

void PhotoDeck::update(const std::vector<fs::path>& photos) {
    const std::set<fs::path> old_set(photos_.begin(), photos_.end());
    const std::set<fs::path> new_set(photos.begin(), photos.end());

    // 消えた写真を残りの山札から取り除く。
    std::erase_if(remaining_, [&](const fs::path& p) { return !new_set.contains(p); });

    // 新しい写真を残りの山札のランダムな位置に差し込む。
    for (const fs::path& p : photos) {
        if (old_set.contains(p)) continue;
        std::uniform_int_distribution<size_t> position(0, remaining_.size());
        remaining_.insert(remaining_.begin() + position(rng_), p);
    }

    photos_ = photos;
}

std::optional<fs::path> PhotoDeck::next() {
    if (photos_.empty()) return std::nullopt;
    if (remaining_.empty()) refill();
    last_drawn_ = remaining_.back();
    remaining_.pop_back();
    return last_drawn_;
}

void PhotoDeck::refill() {
    remaining_ = photos_;
    std::ranges::shuffle(remaining_, rng_);
    // 前の周の最後と新しい周の最初が同じ写真にならないようにする。
    if (remaining_.size() > 1 && remaining_.back() == last_drawn_) std::swap(remaining_.front(), remaining_.back());
}

}  // namespace fotoframe
