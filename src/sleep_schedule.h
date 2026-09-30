#pragma once

namespace fotoframe {

// 消灯する時間帯。時刻は 0:00 からの分数で、開始を含み終了を含まない。
struct SleepHours {
    int start_minutes;
    int end_minutes;
};

bool is_sleep_time(const SleepHours& hours, int minutes_of_day);

}  // namespace fotoframe
