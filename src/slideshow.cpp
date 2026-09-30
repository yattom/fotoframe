#include "slideshow.h"

namespace fotoframe {

Slideshow::Slideshow(double interval_seconds, double transition_seconds, std::optional<SleepHours> sleep_hours)
    : interval_seconds_(interval_seconds), transition_seconds_(transition_seconds), sleep_hours_(sleep_hours) {}

SlideshowFrame Slideshow::update(double now, int minutes_of_day, bool next_ready) {
    const bool sleep_time = sleep_hours_ && is_sleep_time(*sleep_hours_, minutes_of_day);
    if (sleep_time || phase_ == Phase::Sleeping) return update_sleep(now, sleep_time);

    switch (phase_) {
        case Phase::Waiting:
            if (next_ready) return start_transition(now);
            break;
        case Phase::Transitioning: {
            const double elapsed = now - phase_started_;
            if (elapsed < transition_seconds_) {
                return {.phase = phase_, .progress = static_cast<float>(elapsed / transition_seconds_)};
            }
            phase_ = Phase::Showing;
            phase_started_ = now;
            return {.phase = phase_, .finish_transition = true};
        }
        case Phase::Showing:
            if (next_ready && now - phase_started_ >= interval_seconds_) return start_transition(now);
            break;
        case Phase::Sleeping:
            break;
    }
    return {.phase = phase_};
}

SlideshowFrame Slideshow::update_sleep(double now, bool sleep_time) {
    if (phase_ != Phase::Sleeping) {
        // 切り替えの途中なら、終わったことにしてから消灯する。
        const bool was_transitioning = phase_ == Phase::Transitioning;
        phase_ = Phase::Sleeping;
        return {.phase = phase_, .finish_transition = was_transitioning, .enter_sleep = true};
    }
    if (sleep_time) return {.phase = phase_};

    // 消灯明けは、消灯前の写真を表示時間の最初から表示し直す。
    phase_ = has_photo_ ? Phase::Showing : Phase::Waiting;
    phase_started_ = now;
    return {.phase = phase_, .leave_sleep = true};
}

SlideshowFrame Slideshow::start_transition(double now) {
    has_photo_ = true;
    phase_ = Phase::Transitioning;
    phase_started_ = now;
    return {.phase = phase_, .start_transition = true};
}

}  // namespace fotoframe
