#pragma once

#include "shellcode.h"
#include "process.h"

namespace EB
{
    namespace Windows
    {
        using namespace EB::Windows::Process;

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

            // Private Methods
            static bool _write_dll_path(HANDLE const& h_handle, std::wstring const& dll_path, LPVOID& lp_path, size_t* wstr_size_out=nullptr, size_t* buffer_size_out=nullptr);
            static bool _create_thread(HANDLE const& h_handle, LPVOID const& lp_func, LPVOID const& lp_param, ThreadCreationMethod const& thread_creation_method, HANDLE* h_thread_out=nullptr);

            // Public Methods
            static void set_target_process(ExternalProcess* target_process);
            static bool inject_via_loadlibraryw(std::wstring const& dll_path, ThreadCreationMethod const& thread_creation_method=ThreadCreationMethod::CreateRemoteThread);
            // Not implemented yet
            static bool inject_via_ldrloaddll(std::wstring const& dll_path, ThreadCreationMethod const& thread_creation_method=ThreadCreationMethod::CreateRemoteThread);
            // Use it for to load a DLL that will inject the DLL that will perform
            // memory manipulation. Do not forget to check exe name
            static bool inject_via_setwindowshookex(std::wstring const& dll_path, int hook_type=WH_KEYBOARD);
            static bool inject_via_thread_hijacking(std::wstring const& dll_path, unsigned int cleanup_delay_ms=1000);
            static bool inject_dll(InjectionMethod inject_method, std::wstring const& dll_path);
        };
    }
}
