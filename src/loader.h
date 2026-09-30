#pragma once

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

#include "photo_buffer.h"
#include "photo_deck.h"

namespace fotoframe {

// バックグラウンドのスレッドで、写真フォルダの再走査と、NAS から PhotoBuffer への読み込みを行う。
class Loader {
public:
    Loader(std::filesystem::path photo_dir, std::chrono::seconds rescan_interval, PhotoBuffer& buffer,
           unsigned seed);

    // 最後に起きた読み込みのエラー。問題がなければ空。
    std::string last_error() const;

private:
    void run(std::stop_token stop);
    void rescan();
    void set_error(const std::string& error);
    // stop が要求されるか duration が経つまで待つ。止めるときは false。
    bool wait(std::stop_token stop, std::chrono::milliseconds duration);

    std::filesystem::path photo_dir_;
    std::chrono::seconds rescan_interval_;
    PhotoBuffer& buffer_;
    PhotoDeck deck_;

    mutable std::mutex mutex_;
    std::condition_variable_any wake_;
    std::string last_error_;

    // 他のメンバーより後に初期化し、先に破棄する。破棄するときにスレッドへ停止を要求し、終わるのを待つ。
    std::jthread thread_;
};

}  // namespace fotoframe
