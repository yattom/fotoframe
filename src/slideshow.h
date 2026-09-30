#pragma once

#include <optional>

#include "sleep_schedule.h"

namespace fotoframe {

enum class Phase {
    Waiting,        // 表示する写真がまだない
    Showing,        // 写真を表示している
    Transitioning,  // 次の写真へ切り替えている
    Sleeping,       // 消灯中
};

// 1フレームごとの、表示の状態と、このフレームで起きたこと。
struct SlideshowFrame {
    Phase phase;
    float progress = 0;             // Transitioning のときの進み具合（0〜1）
    bool start_transition = false;  // 準備済みの次の写真を「現れる写真」にする
    bool finish_transition = false; // 切り替えが終わった。消えていく写真を手放してよい
    bool enter_sleep = false;       // モニターをスタンバイにする
    bool leave_sleep = false;       // モニターを戻す
};

// 写真の表示時間・切り替え・消灯を、時刻に従って決める。
// 写真そのものは扱わず、呼び出し側に「いつ何をするか」を伝える。
class Slideshow {
public:
    Slideshow(double interval_seconds, double transition_seconds, std::optional<SleepHours> sleep_hours);

    // 毎フレーム呼ぶ。now は単調増加する秒、minutes_of_day は現在時刻（0:00 からの分）、
    // next_ready は次の写真の準備（デコードとテクスチャ作成）ができているか。
    SlideshowFrame update(double now, int minutes_of_day, bool next_ready);

private:
    SlideshowFrame start_transition(double now);
    SlideshowFrame update_sleep(double now, bool sleep_time);

    Phase phase_ = Phase::Waiting;
    bool has_photo_ = false;    // 一度でも写真を表示したか
    double phase_started_ = 0;  // 今の Showing または Transitioning に入った時刻
    double interval_seconds_;
    double transition_seconds_;
    std::optional<SleepHours> sleep_hours_;
};

}  // namespace fotoframe
