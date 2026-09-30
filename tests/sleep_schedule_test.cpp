#include "sleep_schedule.h"

#include <gtest/gtest.h>

namespace fotoframe {
namespace {

constexpr int at(int hour, int minute) { return hour * 60 + minute; }

TEST(SleepScheduleTest, WithinSameDay) {
    const SleepHours hours{at(13, 0), at(15, 0)};

    EXPECT_FALSE(is_sleep_time(hours, at(12, 59)));
    EXPECT_TRUE(is_sleep_time(hours, at(13, 0)));
    EXPECT_TRUE(is_sleep_time(hours, at(14, 59)));
    EXPECT_FALSE(is_sleep_time(hours, at(15, 0)));
}

TEST(SleepScheduleTest, AcrossMidnight) {
    const SleepHours hours{at(23, 0), at(7, 0)};

    EXPECT_FALSE(is_sleep_time(hours, at(22, 59)));
    EXPECT_TRUE(is_sleep_time(hours, at(23, 0)));
    EXPECT_TRUE(is_sleep_time(hours, at(0, 0)));
    EXPECT_TRUE(is_sleep_time(hours, at(6, 59)));
    EXPECT_FALSE(is_sleep_time(hours, at(7, 0)));
    EXPECT_FALSE(is_sleep_time(hours, at(12, 0)));
}

TEST(SleepScheduleTest, SameStartAndEndMeansNeverSleep) {
    const SleepHours hours{at(7, 0), at(7, 0)};

    EXPECT_FALSE(is_sleep_time(hours, at(6, 59)));
    EXPECT_FALSE(is_sleep_time(hours, at(7, 0)));
    EXPECT_FALSE(is_sleep_time(hours, at(0, 0)));
}

}  // namespace
}  // namespace fotoframe
