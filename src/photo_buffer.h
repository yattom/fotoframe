#pragma once

#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "photo_deck.h"

namespace fotoframe {

// バッファから取り出した写真。data はデコード前の JPEG バイト列。
struct BufferedPhoto {
    std::filesystem::path path;
    std::shared_ptr<const std::vector<unsigned char>> data;
};

// これから表示する写真を先読みしておくバッファ。
// loader スレッドが add し、メインスレッドが take するので、各操作は排他制御する。
class PhotoBuffer {
public:
    PhotoBuffer(size_t capacity_bytes, unsigned seed);

    enum class AddResult {
        Added,
        Full,           // 未表示の写真で埋まっていて空きを作れない。時間をおいて再試行する
        TooLarge,       // 容量の上限より大きい。この写真は入れられない
        AlreadyQueued,  // 同じ写真が未表示のまま入っている
    };

    // 写真を未表示として入れる。空きが足りなければ、表示したのが古い写真から捨てる。
    // 表示済みの同じ写真が入っていれば、それを置き換える。
    AddResult add(const std::filesystem::path& path, std::vector<unsigned char> data);

    // 未表示の写真があれば入れた順に出す。なければ表示済みの写真から山札方式で選ぶ。
    std::optional<BufferedPhoto> take();

    size_t used_bytes() const;

private:
    void remove_shown(const std::filesystem::path& path);

    std::optional<BufferedPhoto> take_shown();

    mutable std::mutex mutex_;
    size_t capacity_bytes_;
    size_t used_bytes_ = 0;
    PhotoDeck shown_deck_;               // 未表示の写真がないときに、表示済みの写真から選ぶ
    std::deque<BufferedPhoto> unshown_;  // 先頭から出す
    std::deque<BufferedPhoto> shown_;    // 先頭が、表示したのが最も古いもの
};

}  // namespace fotoframe
