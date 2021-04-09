#pragma once

#include <windows.h>
#include <tlhelp32.h>

#include <string>
#include <vector>

#include "../string.h"

namespace EB
{
    class Process
    {
    public:
        // Structures
        struct ModuleInfo
        {
            std::wstring module_path;      // szExePath
            std::wstring module_name;      // szModule
            BYTE*        module_base_addr; // modBaseAddr
            DWORD        module_base_size; // modBaseSize
            HMODULE      module_handle;    // hModule

            ModuleInfo(std::wstring module_path,      std::wstring module_name,
                       BYTE*        module_base_addr, DWORD        module_base_size,
                       HMODULE      module_handle)
                : module_path(module_path),           module_name(module_name),
                  module_base_addr(module_base_addr), module_base_size(module_base_size),
                  module_handle(module_handle)
            {}
        };

    private:
        // Variables
        PROCESSENTRY32* process_info   = nullptr;
        HANDLE          process_handle = NULL;
        std::vector<ModuleInfo> module_list;

        // Method(s)
        void get_process_info(DWORD const* process_id, std::string const* process_name);
    
    public:
        // Constructor(s)
        Process(std::string const& process_name);
        Process(DWORD const& process_id);

        // Destructor
        ~Process();

        // Method(s)
        PROCESSENTRY32 const* get_process_info() const;
        DWORD          const* get_process_id()   const;
        HANDLE         const* get_handle()       const;

        bool load_module_list();
        std::vector<ModuleInfo> const* get_module_list() const;
    };
}
