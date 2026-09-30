#include "sleep_schedule.h"

namespace fotoframe {

bool is_sleep_time(const SleepHours& hours, int minutes_of_day) {
    const int start = hours.start_minutes;
    const int end = hours.end_minutes;
    if (start <= end) return start <= minutes_of_day && minutes_of_day < end;
    // 日付をまたぐ区間（例：23:00〜7:00）
    return start <= minutes_of_day || minutes_of_day < end;
}

}  // namespace fotoframe
