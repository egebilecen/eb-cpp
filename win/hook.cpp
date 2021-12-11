#include "hook.h"
#include "memory.h"

namespace EB
{
    namespace Windows
    {
        namespace Hook
        {
        #ifdef _WIN64
            constexpr size_t ABSOLUTE_JUMP_HOOK_SIZE = 13;
            static    BYTE   ABSOLUTE_JUMP_HOOK_BYTES[ABSOLUTE_JUMP_HOOK_SIZE] = { 0x49, 0xba, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x41, 0xff, 0xe2 };
        #else
            constexpr size_t RELATIVE_JUMP_HOOK_SIZE = 5;
            static    BYTE   RELATIVE_JUMP_HOOK_BYTES[RELATIVE_JUMP_HOOK_SIZE] = { 0xe9, 0x00, 0x00, 0x00, 0x00 };

            constexpr size_t ABSOLUTE_JUMP_HOOK_SIZE = 8;
            static    BYTE   ABSOLUTE_JUMP_HOOK_BYTES[ABSOLUTE_JUMP_HOOK_SIZE] = { 0x8d, 0x3d, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe7 };
        #endif

        #ifndef _WIN64 // x32
            bool relative_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size)
            {
                if(size < RELATIVE_JUMP_HOOK_SIZE) return false;

                size_t relative_addr = (uintptr_t)func - (uintptr_t)addr - RELATIVE_JUMP_HOOK_SIZE;
                memcpy(RELATIVE_JUMP_HOOK_BYTES + 1, &relative_addr, 4);

                EB::Windows::Memory::write(handle, addr, RELATIVE_JUMP_HOOK_BYTES, RELATIVE_JUMP_HOOK_SIZE);

                if(size > RELATIVE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + RELATIVE_JUMP_HOOK_SIZE, size - RELATIVE_JUMP_HOOK_SIZE);

                return true;
            }

            bool relative_jmp(uintptr_t const& addr, LPVOID func, size_t const& size)
            {
                if(size < RELATIVE_JUMP_HOOK_SIZE) return false;

                size_t relative_addr = (uintptr_t)func - (uintptr_t)addr - RELATIVE_JUMP_HOOK_SIZE;

                memcpy(RELATIVE_JUMP_HOOK_BYTES + 1, &relative_addr, 4);
                EB::Windows::Memory::write(addr, RELATIVE_JUMP_HOOK_BYTES, RELATIVE_JUMP_HOOK_SIZE);

                if(size > RELATIVE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(addr + RELATIVE_JUMP_HOOK_SIZE, size - RELATIVE_JUMP_HOOK_SIZE);

                return true;
            }
        #endif

            bool absolute_jmp(HANDLE const& handle, uintptr_t const& addr, LPVOID func, size_t const& size)
            {
            #ifdef _WIN64
                if(size < ABSOLUTE_JUMP_HOOK_SIZE) return false;

                uintptr_t func_addr = (uintptr_t)func;

                memcpy(ABSOLUTE_JUMP_HOOK_BYTES + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(handle, addr, ABSOLUTE_JUMP_HOOK_BYTES, ABSOLUTE_JUMP_HOOK_SIZE);

                if(size > ABSOLUTE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + ABSOLUTE_JUMP_HOOK_SIZE, size - ABSOLUTE_JUMP_HOOK_SIZE);

                return true;
            #else // x32
                if(size < ABSOLUTE_JUMP_HOOK_SIZE) return false;

                uintptr_t func_addr = (uintptr_t)func;

                memcpy(ABSOLUTE_JUMP_HOOK_BYTES + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(handle, addr, ABSOLUTE_JUMP_HOOK_BYTES, ABSOLUTE_JUMP_HOOK_SIZE);

                if(size > ABSOLUTE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(handle, addr + ABSOLUTE_JUMP_HOOK_SIZE, size - ABSOLUTE_JUMP_HOOK_SIZE);

                return true;
            #endif
            }

            bool absolute_jmp(uintptr_t const& addr, LPVOID func, size_t const& size)
            {
            #ifdef _WIN64
                if(size < ABSOLUTE_JUMP_HOOK_SIZE) return false;

                uintptr_t func_addr = (uintptr_t)func;

                memcpy(ABSOLUTE_JUMP_HOOK_BYTES + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(addr, ABSOLUTE_JUMP_HOOK_BYTES, ABSOLUTE_JUMP_HOOK_SIZE);

                if(size > ABSOLUTE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(addr + ABSOLUTE_JUMP_HOOK_SIZE, size - ABSOLUTE_JUMP_HOOK_SIZE);

                return true;
            #else // x32
                if(size < ABSOLUTE_JUMP_HOOK_SIZE) return false;

                uintptr_t func_addr = (uintptr_t)func;

                memcpy(ABSOLUTE_JUMP_HOOK_BYTES + 2, &func_addr, sizeof(uintptr_t));
                EB::Windows::Memory::write(addr, ABSOLUTE_JUMP_HOOK_BYTES, ABSOLUTE_JUMP_HOOK_SIZE);

                if(size > ABSOLUTE_JUMP_HOOK_SIZE) 
                    EB::Windows::Memory::fill_with_nop(addr + ABSOLUTE_JUMP_HOOK_SIZE, size - ABSOLUTE_JUMP_HOOK_SIZE);

                return true;
            #endif
            }

        #ifdef _WIN64
            // Be sure to build DLL in DEBUG mode. This function won't work on RELEASE.
            bool absolute_jmp_and_restore(uintptr_t const& addr, LPVOID func, size_t const& size)
            {
                if(size < ABSOLUTE_JUMP_HOOK_SIZE) return false;

                uintptr_t func_addr = EB::Windows::Memory::get_function_real_address(func);
                size_t    func_size = EB::Windows::Memory::get_function_size(func);

                if(func_addr == 0x00
                || func_size == 0x00) return false;

                BYTE* bytes_to_restore = new BYTE[size];
                byte  ret_instruction  = 0xC3;
                DWORD old_protect;

                memcpy(bytes_to_restore, (void*)addr, size);

                VirtualProtect((void*)func_addr, func_size + size, PAGE_EXECUTE_READWRITE, &old_protect);

                memcpy((void*)(func_addr + func_size - 1), bytes_to_restore, size);
                *(BYTE*)(func_addr + func_size + size - 1) = 0xC3;

                VirtualProtect((void*)func_addr, func_size + size, old_protect, &old_protect);

                delete[] bytes_to_restore;

                if(!absolute_jmp(addr, func, size))
                    return false;

                return true;
            }
        #endif
            
            bool IAT(std::string const& module_name, std::string const& func_name, LPVOID new_func, LPVOID out_old_func) 
            {
                PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)GetModuleHandleW(NULL);
                PIMAGE_NT_HEADERS nt_header  = (PIMAGE_NT_HEADERS)((uintptr_t)dos_header + dos_header->e_lfanew);

                if(nt_header->Signature != IMAGE_NT_SIGNATURE)
                    return false;

                PIMAGE_IMPORT_DESCRIPTOR import_descriptor = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)dos_header + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

                for(size_t i=0; import_descriptor[i].Characteristics != 0; i++)
                {
                    std::string dll_name = std::string((char*)((uintptr_t)dos_header + import_descriptor[i].Name));

                    if(module_name != EB::String::lowercase(dll_name))
                        continue;

                    if(!import_descriptor[i].FirstThunk || !import_descriptor[i].OriginalFirstThunk)
                        return false;

                    PIMAGE_THUNK_DATA thunk_data          = (PIMAGE_THUNK_DATA)((uintptr_t)dos_header + import_descriptor[i].FirstThunk);
                    PIMAGE_THUNK_DATA original_thunk_data = (PIMAGE_THUNK_DATA)((uintptr_t)dos_header + import_descriptor[i].OriginalFirstThunk);

                    for(; original_thunk_data->u1.Function != NULL; original_thunk_data++, thunk_data++)
                    {
                        if(original_thunk_data->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                            continue;

                        PIMAGE_IMPORT_BY_NAME import_data = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)dos_header + original_thunk_data->u1.AddressOfData);
                        std::string import_data_name = std::string((char*)import_data->Name);

                        if(func_name != import_data_name)
                            continue;

                        DWORD junk = 0;
                        MEMORY_BASIC_INFORMATION mbi;

                        VirtualQuery(thunk_data, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
                        if(!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect))
                            return false;

                        if(out_old_func != NULL)
                            out_old_func = (void**)((uintptr_t)thunk_data->u1.Function);

                        thunk_data->u1.Function = (uintptr_t)new_func;

                        if(VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &junk))
                            return true;
                    }
                }

                return true;
            }
        }
    }
}
