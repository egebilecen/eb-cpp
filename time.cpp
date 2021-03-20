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
    }
}
