#include "keyboard.h"

namespace EB
{
    namespace Windows
    {
        namespace Keyboard
        {
            std::int16_t get_key_down_async()
            {
                for(int i=VIRTUAL_KEY_BEGIN; i <= VIRTUAL_KEY_END; i++)
                    if(GetAsyncKeyState(i) & 0x8000) return i;

                return 0;
            }

            bool get_is_key_down_async(uint16_t vkey)
            {
                if(GetAsyncKeyState(vkey) & 0x8000) return true;

                return false;
            }
        }
    }
}
