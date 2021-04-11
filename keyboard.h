#pragma once

#include "Windows.h"
#include <cstdint>

namespace EB
{
    namespace Windows
    {
        namespace Keyboard
        {
            const int VIRTUAL_KEY_BEGIN = 0x08;
            const int VIRTUAL_KEY_END   = 0x7B;

            std::int16_t get_key_down_async();
            bool get_is_key_down_async(uint16_t vkey);
        }
    }
}
