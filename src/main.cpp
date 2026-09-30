// fotoframe: NAS 上の写真をシャッフルしながらフルスクリーンで表示するデジタルフォトフレーム。
// 使い方: fotoframe [設定ファイル]（省略時は ~/.config/fotoframe/config.toml）

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <random>
#include <thread>

#include "config.h"
#include "display_power.h"
#include "jpeg_decoder.h"
#include "loader.h"
#include "photo_buffer.h"
#include "presentation.h"
#include "renderer.h"
#include "slideshow.h"
#include "transition.h"

namespace fotoframe {
namespace {

using namespace std::chrono_literals;

constexpr size_t kMegabyte = 1024 * 1024;
constexpr auto kIdleFrameWait = 100ms;   // 切り替え中以外は描画の頻度を下げる
constexpr auto kSleepingWait = 1s;       // 消灯中の待ち時間

std::filesystem::path default_config_path() {
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / ".config/fotoframe/config.toml";
}

int minutes_of_day_now() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    return local.tm_hour * 60 + local.tm_min;
}

// バッファから次の写真を取り出し、デコードして GPU に転送する。準備できなければ nullptr。
std::unique_ptr<Slide> prepare_next(PhotoBuffer& buffer, JpegDecoder& decoder, PresentationChooser& chooser,
                                    Renderer& renderer) {
    const auto photo = buffer.take();
    if (!photo) return nullptr;
    const Presentation presentation = chooser.next();
    const auto image = decoder.decode(*photo->data, presentation.mode, renderer.screen());
    if (!image) {
        std::fprintf(stderr, "cannot decode %s\n", photo->path.c_str());
        return nullptr;
    }
    std::printf("next: %s\n", photo->path.c_str());
    return renderer.make_slide(*image, presentation);
}

int run(const Config& config) {
    const auto renderer = Renderer::create();
    if (!renderer) return 1;

    std::random_device seed;
    PhotoBuffer buffer(static_cast<size_t>(config.buffer_megabytes) * kMegabyte, seed());
    const auto rescan_interval = std::chrono::seconds(static_cast<long>(config.rescan_minutes * 60));
    Loader loader(config.photo_dir, rescan_interval, buffer, seed());
    PresentationChooser chooser(config.display_mode, config.effects, seed());
    Slideshow slideshow(config.interval_seconds, config.transition_seconds, config.sleep_hours);
    JpegDecoder decoder;

    std::unique_ptr<Slide> current;   // 表示している写真（切り替え中は現れる写真）
    std::unique_ptr<Slide> outgoing;  // 切り替え中に消えていく写真
    std::unique_ptr<Slide> next;      // 次に表示する写真

    const auto start = std::chrono::steady_clock::now();
    while (!renderer->quit_requested()) {
        const double now = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        const SlideshowFrame frame = slideshow.update(now, minutes_of_day_now(), next != nullptr);

        if (frame.enter_sleep) set_display_power(false);
        if (frame.leave_sleep) set_display_power(true);
        if (frame.start_transition) {
            outgoing = std::move(current);
            current = std::move(next);
        }
        if (frame.finish_transition) outgoing.reset();

        if (frame.phase == Phase::Sleeping) {
            std::this_thread::sleep_for(kSleepingWait);
            continue;
        }

        renderer->clear();
        if (frame.phase == Phase::Transitioning) {
            const TransitionFrame t = transition_frame(current->presentation().effect, frame.progress, renderer->screen());
            if (outgoing) renderer->draw_slide(*outgoing, t.outgoing);
            renderer->draw_slide(*current, t.incoming);
        } else if (current) {
            renderer->draw_slide(*current, {1, 0, 0});
        } else if (const std::string error = loader.last_error(); !error.empty()) {
            renderer->draw_message(error);
        }
        renderer->present();

        if (frame.phase != Phase::Transitioning) {
            // 切り替え中は描画を止めないように、次の写真の準備は表示中か待機中にだけ行う。
            if (!next) next = prepare_next(buffer, decoder, chooser, *renderer);
            std::this_thread::sleep_for(kIdleFrameWait);
        }
    }
    return 0;
}

}  // namespace
}  // namespace fotoframe

int main(int argc, char* argv[]) {
    // ファイルやパイプへ出力するときもログがすぐ出るように行単位でフラッシュする。
    std::setvbuf(stdout, nullptr, _IOLBF, 0);

    const std::filesystem::path config_path = (argc >= 2) ? argv[1] : fotoframe::default_config_path();
    const fotoframe::ConfigResult result = fotoframe::load_config(config_path);
    if (!result.config) {
        std::fprintf(stderr, "%s\n", result.error.c_str());
        return 1;
    }
    return fotoframe::run(*result.config);
}
