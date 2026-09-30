#include "loader.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <optional>

#include "photo_scanner.h"

namespace fotoframe {

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace {

constexpr auto kRescanRetryInterval = 1min;  // 走査に失敗したときの再試行の間隔
constexpr auto kReadRetryInterval = 10s;     // 写真を読めなかったときの待ち時間
constexpr auto kBufferFullWait = 2s;         // バッファに空きがないときの待ち時間

std::optional<std::vector<unsigned char>> read_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return std::nullopt;
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (in.bad()) return std::nullopt;
    return data;
}

}  // namespace

Loader::Loader(fs::path photo_dir, std::chrono::seconds rescan_interval, PhotoBuffer& buffer, unsigned seed)
    : photo_dir_(std::move(photo_dir)),
      rescan_interval_(rescan_interval),
      buffer_(buffer),
      deck_(seed),
      thread_([this](std::stop_token stop) { run(stop); }) {}

std::string Loader::last_error() const {
    std::lock_guard lock(mutex_);
    return last_error_;
}

void Loader::set_error(const std::string& error) {
    std::lock_guard lock(mutex_);
    if (!error.empty() && error != last_error_) std::fprintf(stderr, "loader: %s\n", error.c_str());
    last_error_ = error;
}

bool Loader::wait(std::stop_token stop, std::chrono::milliseconds duration) {
    std::unique_lock lock(mutex_);
    wake_.wait_for(lock, stop, duration, [] { return false; });
    return !stop.stop_requested();
}

void Loader::run(std::stop_token stop) {
    auto next_scan = std::chrono::steady_clock::now();
    std::optional<std::pair<fs::path, std::vector<unsigned char>>> pending;  // 読んだがバッファに入れていない写真

    while (!stop.stop_requested()) {
        if (std::chrono::steady_clock::now() >= next_scan) {
            rescan();
            next_scan = std::chrono::steady_clock::now() + (last_error().empty() ? rescan_interval_ : kRescanRetryInterval);
        }

        if (!pending) {
            const auto path = deck_.next();
            if (!path) {
                if (!wait(stop, kReadRetryInterval)) break;
                continue;
            }
            auto data = read_file(*path);
            if (!data) {
                set_error("cannot read " + path->string());
                if (!wait(stop, kReadRetryInterval)) break;
                continue;
            }
            pending.emplace(*path, std::move(*data));
        }

        switch (buffer_.add(pending->first, pending->second)) {
            case PhotoBuffer::AddResult::Added:
                set_error("");
                pending.reset();
                break;
            case PhotoBuffer::AddResult::TooLarge:
                std::fprintf(stderr, "loader: too large for buffer: %s\n", pending->first.c_str());
                pending.reset();
                break;
            case PhotoBuffer::AddResult::AlreadyQueued:
                // 写真が少なく、すべてバッファに入っている
                pending.reset();
                if (!wait(stop, kBufferFullWait)) return;
                break;
            case PhotoBuffer::AddResult::Full:
                if (!wait(stop, kBufferFullWait)) return;
                break;
        }
    }
}

void Loader::rescan() {
    const auto start = std::chrono::steady_clock::now();
    const auto photos = scan_photos(photo_dir_);
    if (!photos) {
        set_error("cannot read " + photo_dir_.string());
        return;
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::printf("loader: found %zu photos in %lld ms\n", photos->size(), static_cast<long long>(elapsed.count()));
    deck_.update(*photos);
    set_error(photos->empty() ? "no photos in " + photo_dir_.string() : "");
}

}  // namespace fotoframe
