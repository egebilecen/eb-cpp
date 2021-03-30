#include "time.h"

namespace EB
{
    namespace Time
    {
        std::tm convert_date_to_tm(Date const& date)
        {
            struct std::tm d {
                date.second, date.minute, date.hour,
                date.day, date.month - 1, date.year - 1900
            };

            return d;
        }

        long long get_time_since_epoch_ms()
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

        // Be sure date2 > date1
        double get_difference_between_dates(Date const& date1, Date const& date2)
        {
            struct std::tm d1 = convert_date_to_tm(date1);
            struct std::tm d2 = convert_date_to_tm(date2);

            std::time_t t1 = std::mktime(&d1);
            std::time_t t2 = std::mktime(&d2);

            double diff_secs = std::difftime(t2, t1);

            return diff_secs;
        }
    }
}
