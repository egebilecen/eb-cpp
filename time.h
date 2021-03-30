#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>

#define ONE_YEAR_IN_SECONDS  31556926
#define ONE_MONTH_IN_SECONDS 2629743
#define ONE_DAY_IN_SECONDS   86400

namespace EB
{
    namespace Time
    {
        struct Date
        {
            int day    = 0;
            int month  = 0;
            int year   = 0;

            int hour   = 0;
            int minute = 0;
            int second = 0;
        };

        std::tm convert_date_to_tm(Date const& date);
        long long get_time_since_epoch_ms();
        Date get_local_date();
        double get_difference_between_dates(Date const& date1, Date const& date2);
    }
}
