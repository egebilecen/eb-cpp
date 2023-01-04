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
        #ifdef _WIN64
            constexpr size_t ABSOLUTE_JUMP_HOOK_SIZE = 13;
        #else
            constexpr size_t RELATIVE_JUMP_HOOK_SIZE = 5;

            constexpr size_t ABSOLUTE_JUMP_HOOK_SIZE = 8;
        #endif

            uintptr_t trampoline(uintptr_t const& from_addr, uintptr_t const& target_addr, size_t const& size);

        #ifndef _WIN64
            bool relative_jmp(HANDLE const& handle, uintptr_t const& from_addr, uintptr_t const& target_addr, size_t const& size);
            bool relative_jmp(uintptr_t const& from_addr, uintptr_t const& target_addr, size_t const& size);
        #endif
            bool absolute_jmp(HANDLE const& handle, uintptr_t const& from_addr, uintptr_t const& target_addr, size_t const& size);
            bool absolute_jmp(uintptr_t const& from_addr, uintptr_t const& target_addr, size_t const& size);

        #ifdef _WIN64
            bool absolute_jmp_and_restore(uintptr_t const& from_addr, LPVOID target_addr, size_t const& size);
        #endif

            bool IAT(std::string const& module_name, std::string const& func_name, LPVOID new_func, LPVOID out_old_func=NULL);
        }
    }
}
