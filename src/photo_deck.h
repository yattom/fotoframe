#pragma once

#include <filesystem>
#include <optional>
#include <random>
#include <vector>

namespace fotoframe {

// 山札方式で写真を選ぶ。全部を一巡するまで同じ写真を出さず、一巡したら並べ直す。
class PhotoDeck {
public:
    explicit PhotoDeck(unsigned seed);

    // 写真の一覧を置き換える。
    void update(const std::vector<std::filesystem::path>& photos);

    // 次の写真を引く。写真が1枚もなければ nullopt。
    std::optional<std::filesystem::path> next();

private:
    void refill();

    std::mt19937 rng_;
    std::vector<std::filesystem::path> photos_;
    std::vector<std::filesystem::path> remaining_;  // 末尾から引く
    std::filesystem::path last_drawn_;
};

}  // namespace fotoframe
