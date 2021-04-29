#include "hook.h"
#include "memory.h"

namespace EB
{
    namespace Windows
    {
        namespace Hook
        {
        #ifndef _WIN64 // x32
            bool relative_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size)
            {
                constexpr size_t hook_size = 5;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size] = { 0xe9, 0x00, 0x00, 0x00, 0x00 };

                size_t relative_addr = (uintptr_t)func - (uintptr_t)addr - sizeof(hook_bytes);

                memcpy(hook_bytes + 1, &relative_addr, 4);

                EB::Windows::Memory::write(handle, addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + hook_size, size - hook_size);

                return true;
            }

            bool relative_jmp(uintptr_t const& addr, LPVOID func, size_t const& size)
            {
                constexpr size_t hook_size = 5;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size] = { 0xe9, 0x00, 0x00, 0x00, 0x00 };

                size_t relative_addr = (uintptr_t)func - (uintptr_t)addr - sizeof(hook_bytes);

                memcpy(hook_bytes + 1, &relative_addr, 4);
                EB::Windows::Memory::write(addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(addr + hook_size, size - hook_size);

                return true;
            }
        #endif

            bool absolute_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size)
            {
            #ifdef _WIN64
                constexpr size_t hook_size = 13;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size]  = { 0x49, 0xba, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x41, 0xff, 0xe2 };
                uintptr_t func_addr = (uintptr_t)func;

                memcpy(hook_bytes + 2, &func_addr, sizeof(uintptr_t));

                EB::Windows::Memory::write(handle, addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + hook_size, size - hook_size);

                return true;
            #else // x32
                constexpr size_t hook_size = 8;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size]  = { 0x8d, 0x3d, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe7 };
                uintptr_t func_addr = (uintptr_t)func;

                memcpy(hook_bytes + 2, &func_addr, sizeof(uintptr_t));

                EB::Windows::Memory::write(handle, addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + hook_size, size - hook_size);

                return true;
            #endif
            }

            bool absolute_jmp(uintptr_t const& addr, LPVOID func, size_t const& size)
            {
            #ifdef _WIN64
                constexpr size_t hook_size = 13;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size]  = { 0x49, 0xba, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x41, 0xff, 0xe2 };
                uintptr_t func_addr = (uintptr_t)func;

                memcpy(hook_bytes + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + hook_size, size - hook_size);

                return true;
            #else // x32
                constexpr size_t hook_size = 8;

                if(size < hook_size) return false;

                BYTE hook_bytes[hook_size]  = { 0x8d, 0x3d, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe7 };
                uintptr_t func_addr = (uintptr_t)func;

                memcpy(hook_bytes + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(addr, hook_bytes, hook_size);

                if(size > hook_size) 
                    EB::Windows::Memory::fill_with_nop(addr + hook_size, size - hook_size);

                return true;
            #endif
            }
        }
    }
}
