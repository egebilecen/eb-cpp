#pragma once

#include <Windows.h>
#include <cstdint>

namespace EB
{
    namespace Windows
    {
        namespace Hook
        {
            namespace x86
            {
                bool relative_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size);
                bool absolute_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size);
            }

            namespace x64
            {
                bool absolute_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size);
            }
        }
    }
}
