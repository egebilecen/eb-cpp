#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

#include "eb/string.h"

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

        #ifdef _WIN64
            bool absolute_jmp_and_restore(uintptr_t const& addr, LPVOID func, size_t const& size);
        #endif

            bool IAT(std::string const& module_name, std::string const& func_name, LPVOID new_func, LPVOID out_old_func=NULL);
        }
    }
}
