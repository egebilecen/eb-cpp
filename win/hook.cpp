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
