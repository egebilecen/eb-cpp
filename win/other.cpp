#include "other.h"
#include <cstdio>

namespace EB
{
    namespace Windows
    {
        namespace Other
        {
            void allocate_console()
            {
                AllocConsole();
                freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
                freopen_s((FILE**)stdout, "CONIN$" , "r", stdin);
            }

            void deallocate_console()
            {
                FreeConsole();
            }
        }
    }
}
