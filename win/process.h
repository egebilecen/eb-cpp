#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <SubAuth.h>

#include <string>
#include <vector>

#include "shellcode.h"
#include "../string.h"

namespace EB
{
    namespace Process
    {
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

        struct ThreadInfo
        {
            DWORD thread_id;
            DWORD thread_owner_process_id;
            LONG  thread_priority_level;

            ThreadInfo(DWORD thread_id, DWORD thread_owner_process_id, LONG thread_priority_level)
                : thread_id(thread_id), thread_owner_process_id(thread_owner_process_id), 
                  thread_priority_level(thread_priority_level)
            {}
        };

        class ExternalProcess
        {

        private:
            // Variables
            PROCESSENTRY32W* process_info   = nullptr;
            HANDLE           process_handle = NULL;
            std::vector<ModuleInfo> module_list;
            std::vector<ThreadInfo> thread_list;

            // Method(s)
            void get_process_info(DWORD const* process_id, std::string const* process_name);

        public:
            // Constructor(s)
            ExternalProcess(std::string const& process_name);
            ExternalProcess(DWORD const& process_id);

            // Destructor
            ~ExternalProcess();

            // Method(s)
            PROCESSENTRY32W const* get_process_info() const;
            DWORD           const* get_process_id()   const;
            HANDLE get_process_handle() const;

            bool load_module_list();
            bool load_thread_list();
            std::vector<ModuleInfo> const* get_module_list() const;
            // TODO: Add support for NtQuerySystemInformation() to get thread list.
            std::vector<ThreadInfo> const* get_thread_list() const;
            ModuleInfo const* get_module(std::string const& module_name) const;
        };

        namespace InternalProcess
        {
            DWORD get_process_id();
            HANDLE get_process_handle();
            HMODULE get_module_handle();
            std::vector<ModuleInfo> get_module_list();
            ModuleInfo const* get_module(std::vector<ModuleInfo> const* module_list, std::string const& module_name);
        }

        static class Injector
        {
        public:
            enum class InjectionMethod
            {
                LoadLibraryW,
                LdrLoadDll,
                SetWindowsHookExW,
                ThreadHijacking
            };

            static ExternalProcess* target_process;

            static void set_target_process(ExternalProcess* target_process);
            static bool inject_via_loadlibraryw(std::string const& dll_path);
            // Not implemented yet
            static bool inject_via_ldrloaddll(std::string const& dll_path);
            // Use it for to load a DLL that will inject the DLL that will perform
            // memory manipulation. Do not forget to check exe name
            static bool inject_via_setwindowshookex(std::string const& dll_path, int hook_type=WH_KEYBOARD);
            static bool inject_via_thread_hijacking(std::string const& dll_path);
            static bool inject_dll(InjectionMethod inject_method, std::string const& dll_path);
        };
    }
}
