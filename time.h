#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>

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

        std::int32_t get_time_since_epoch_ms();
        Date get_local_date();
    }
}
