#pragma once

#include <Windows.h>
#include <cstdint>

namespace EB
{
    namespace Windows
    {
        namespace Hook
        {
        #ifndef _WIN64
            bool relative_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size);
            bool relative_jmp(uintptr_t const& addr, LPVOID func, size_t const& size);
        #endif
            bool absolute_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size);
            bool absolute_jmp(uintptr_t const& addr, LPVOID func, size_t const& size);
        }
    }
}
