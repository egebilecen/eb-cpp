#include "time.h"

namespace EB
{
    namespace Time
    {
        std::int32_t get_time_since_epoch_ms()
        {
            using namespace std::chrono;

            return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        }

        Date get_local_date()
        {
            using namespace std::chrono;

            system_clock::time_point now = system_clock::now();
            std::time_t now_c = system_clock::to_time_t(now);
            struct tm* parts = std::localtime(&now_c);

            Date date {
                parts->tm_mday, 1 + parts->tm_mon, 1900 + parts->tm_year,
                parts->tm_hour, parts->tm_min, parts->tm_sec
            };

            return date;
        }
    }
}
