#include "injector.h"

namespace EB
{
    namespace Windows
    {
        ExternalProcess* Injector::target_process = nullptr;

        bool Injector::_write_dll_path(HANDLE const& h_handle, std::wstring const& dll_path, LPVOID& lp_path, size_t* wstr_size_out, size_t* buffer_size_out)
        {
            if(Injector::target_process == nullptr) return false;

            WCHAR dll_path_str[MAX_PATH];
            ZeroMemory(dll_path_str, sizeof(dll_path_str));
            dll_path.copy(dll_path_str, MAX_PATH);

            if(buffer_size_out != nullptr)
                *buffer_size_out = sizeof(dll_path_str);

            if(wstr_size_out != nullptr)
                StringCbLengthW(dll_path_str, sizeof(dll_path_str), wstr_size_out);

            LPVOID lp_dll_path = VirtualAllocEx(h_handle, NULL, sizeof(dll_path_str), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            if(!lp_dll_path) return false;

            if(!WriteProcessMemory(h_handle, lp_dll_path, dll_path_str, sizeof(dll_path_str), NULL))
                return false;

            lp_path = lp_dll_path;
            return true;
        }

        bool Injector::_create_thread(HANDLE const& h_handle, LPVOID const& lp_func, LPVOID const& lp_param, ThreadCreationMethod const& thread_creation_method, HANDLE* h_thread_out)
        {
            if(Injector::target_process == nullptr) return false;

            switch(thread_creation_method)
            {
                case ThreadCreationMethod::CreateRemoteThread:
                {
                    HANDLE h_thread = CreateRemoteThread(h_handle, NULL, NULL, 
                                                         (LPTHREAD_START_ROUTINE)lp_func, lp_param, 
                                                         NULL, NULL);

                    if(!h_thread) return false;
                    if(h_thread_out != nullptr) *h_thread_out = h_thread;
                }
                break;

                case ThreadCreationMethod::NtCreateThreadEx:
                {
                    LPVOID lp_ntcreatethreadex = GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateThreadEx");

                    if(!lp_ntcreatethreadex) return false;

                    HANDLE h_thread = NULL;

                    ((_NtCreateThreadEx)(lp_ntcreatethreadex))(&h_thread, THREAD_ALL_ACCESS, NULL,
                                                               h_handle, (LPTHREAD_START_ROUTINE)lp_func, 
                                                               lp_param, FALSE, NULL, NULL, NULL, NULL);

                    if(!h_thread) return false;
                    if(h_thread_out != nullptr) *h_thread_out = h_thread;
                }
                break;

                case ThreadCreationMethod::RtlCreateUserThread:
                {
                    LPVOID lp_rtlcreateuserthread = GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlCreateUserThread");

                    if(!lp_rtlcreateuserthread) return false;

                    HANDLE h_thread = NULL;

                    ((_RtlCreateUserThread)(lp_rtlcreateuserthread))(h_handle, 
                                                                     NULL, FALSE, NULL, NULL, NULL, 
                                                                     lp_func, lp_param, &h_thread, NULL);

                    if(!h_thread) return false;
                    if(h_thread_out != nullptr) *h_thread_out = h_thread;
                }
                break;
            }

            return true;
        }

        void Injector::set_target_process(ExternalProcess* target_process)
        {
            Injector::target_process = target_process;
        }

        bool Injector::inject_via_loadlibraryw(std::wstring const& dll_path, ThreadCreationMethod const& thread_creation_method)
        {
            if(Injector::target_process == nullptr) return false;

            LPVOID lp_loadlibraryw = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

            if(!lp_loadlibraryw) return false;

            HANDLE h_target_process = Injector::target_process->get_process_handle();

            if(!h_target_process) return false;

            LPVOID lp_dll_path = NULL;

            if(!Injector::_write_dll_path(h_target_process, dll_path, lp_dll_path))
            {
                CloseHandle(h_target_process);
                return false;
            }

            HANDLE h_thread = NULL;

            if(!Injector::_create_thread(h_target_process, lp_loadlibraryw, lp_dll_path, thread_creation_method, &h_thread))
            {
                VirtualFreeEx(h_target_process, lp_dll_path, 0, MEM_RELEASE);
                CloseHandle(h_target_process);
                return false;
            }

            DWORD thread_exit_code;

            do
            {
                GetExitCodeThread(h_thread, &thread_exit_code);
            }
            while(thread_exit_code == STILL_ACTIVE);

            VirtualFreeEx(h_target_process, lp_dll_path,     0, MEM_RELEASE);

            CloseHandle(h_target_process);
            CloseHandle(h_thread);

            return true;
        }

        bool Injector::inject_via_ldrloaddll(std::wstring const& dll_path, ThreadCreationMethod const& thread_creation_method)
        {
            if(Injector::target_process == nullptr) return false;

            LPVOID lp_ldrloaddll = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "LdrLoadDll");

            if(!lp_ldrloaddll) return false;

            HANDLE h_target_process = Injector::target_process->get_process_handle();

            if(!h_target_process) return false;

            LPVOID lp_dll_path = NULL;
            size_t wstr_size   = 0;
            size_t buffer_size = 0;

            if(!Injector::_write_dll_path(h_target_process, dll_path, lp_dll_path, &wstr_size, &buffer_size))
            {
                CloseHandle(h_target_process);
                return false;
            }

            UNICODE_STRING dll_path_unicode;
            dll_path_unicode.Buffer        = (WCHAR*)lp_dll_path;
            dll_path_unicode.Length        = wstr_size;
            dll_path_unicode.MaximumLength = buffer_size;

            LPVOID lp_unicode_str = VirtualAllocEx(h_target_process, NULL, sizeof(dll_path_unicode), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            WriteProcessMemory(h_target_process, lp_unicode_str, &dll_path_unicode, sizeof(dll_path_unicode), NULL);

            HANDLE h_out = NULL;
            LPVOID lp_handle_out = VirtualAllocEx(h_target_process, NULL, sizeof(h_out), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            WriteProcessMemory(h_target_process, lp_handle_out, &h_out, sizeof(h_out), NULL);

            BYTE shellcode[sizeof(shellcode_x32_ldrloaddll)];
            memcpy(shellcode, shellcode_x32_ldrloaddll, sizeof(shellcode_x32_ldrloaddll));

            *((void**)(shellcode + 4))  = lp_handle_out;
            *((void**)(shellcode + 9))  = lp_unicode_str;
            *((void**)(shellcode + 18)) = lp_ldrloaddll;

            LPVOID lp_shellcode = VirtualAllocEx(h_target_process, NULL, sizeof(shellcode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            WriteProcessMemory(h_target_process, lp_shellcode, shellcode, sizeof(shellcode), NULL);

            HANDLE h_thread = NULL;

            if(!Injector::_create_thread(h_target_process, lp_shellcode, lp_dll_path, thread_creation_method, &h_thread))
            {
                VirtualFreeEx(h_target_process, lp_dll_path,    0, MEM_RELEASE);
                VirtualFreeEx(h_target_process, lp_handle_out,  0, MEM_RELEASE);
                VirtualFreeEx(h_target_process, lp_unicode_str, 0, MEM_RELEASE);
                VirtualFreeEx(h_target_process, lp_shellcode,   0, MEM_RELEASE);

                CloseHandle(h_target_process);

                return false;
            }

            DWORD thread_exit_code;

            do
            {
                GetExitCodeThread(h_thread, &thread_exit_code);
            }
            while(thread_exit_code == STILL_ACTIVE);

            // Cleanup
            VirtualFreeEx(h_target_process, lp_dll_path,    0, MEM_RELEASE);
            VirtualFreeEx(h_target_process, lp_handle_out,  0, MEM_RELEASE);
            VirtualFreeEx(h_target_process, lp_unicode_str, 0, MEM_RELEASE);
            VirtualFreeEx(h_target_process, lp_shellcode,   0, MEM_RELEASE);

            CloseHandle(h_target_process);
            CloseHandle(h_thread);

            return true;
        }

        bool Injector::inject_via_setwindowshookex(std::wstring const& dll_path, int hook_type)
        {
            HMODULE dll = LoadLibraryW(dll_path.c_str());

            if(!dll) return false;

            HOOKPROC addr = (HOOKPROC)GetProcAddress(dll, "HookMain");

            if(!addr) return false;

            HHOOK hook_handle = SetWindowsHookExW(hook_type, addr, dll, 0);

            if(!hook_handle) return false;

            // UnhookWindowsHookEx(hook_handle);

            return true;
        }

        bool Injector::inject_via_thread_hijacking(std::wstring const& dll_path, unsigned int cleanup_delay_ms)
        {
            if(Injector::target_process == nullptr) return false;

            Injector::target_process->load_thread_list();
            std::vector<ThreadInfo> const* thread_list = Injector::target_process->get_thread_list();

            if(thread_list->size() < 1) return false;

            HANDLE h_thread = OpenThread(THREAD_ALL_ACCESS, NULL, (*thread_list)[0].thread_id);

            if(!h_thread) return false;

            LPVOID lp_loadlibraryw = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

            if(!lp_loadlibraryw) return false;

        #ifdef _WIN64
            BYTE shell_code64[sizeof(shellcode_x64_thread_hijacking)];
            memcpy(shell_code64, shellcode_x64_thread_hijacking, sizeof(shellcode_x64_thread_hijacking));
        #else
            BYTE shellcode32[sizeof(shellcode_x32_thread_hijacking)];
            memcpy(shellcode32, shellcode_x32_thread_hijacking, sizeof(shellcode_x32_thread_hijacking));
        #endif

            HANDLE h_target_process = Injector::target_process->get_process_handle();

            // Write DLL name to target process' memory
            LPVOID lp_dll_path = NULL;

            if(!Injector::_write_dll_path(h_target_process, dll_path, lp_dll_path))
            {
                CloseHandle(h_target_process);
                return false;
            }

            SuspendThread(h_thread);

            CONTEXT context;
            context.ContextFlags = CONTEXT_FULL;

            GetThreadContext(h_thread, &context);

            // Update shellcode addresses
        #ifdef _WIN64
            // Set dll path
            *((void**)(shell_code64 + 0)) = lp_dll_path;
            // Set loadlibraryw
            *((void**)(shell_code64 + 0)) = lp_loadlibraryw;
            // Set IP
            *((DWORD*)(shell_code64 + 0)) = context.Rip;

            LPVOID lp_shellcode = VirtualAllocEx(h_target_process, NULL, sizeof(shell_code64), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            WriteProcessMemory(h_target_process, lp_shellcode, shell_code64, sizeof(shell_code64), NULL);

            context.Rip = (DWORD_PTR)lp_shellcode;
        #else
            // Set dll path
            *((void**)(shellcode32 + 9))  = lp_dll_path;
            // Set loadlibraryw
            *((void**)(shellcode32 + 15)) = lp_loadlibraryw;
            // Set IP
            *((DWORD*)(shellcode32 + 28)) = context.Eip;

            LPVOID lp_shellcode = VirtualAllocEx(h_target_process, NULL, sizeof(shellcode32), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            WriteProcessMemory(h_target_process, lp_shellcode, shellcode32, sizeof(shellcode32), NULL);

            context.Eip = (DWORD)lp_shellcode;
        #endif

            SetThreadContext(h_thread, &context);
            ResumeThread(h_thread);

            if(cleanup_delay_ms > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(cleanup_delay_ms));

                VirtualFreeEx(h_target_process, lp_dll_path,  0, MEM_RELEASE);
                VirtualFreeEx(h_target_process, lp_shellcode, 0, MEM_RELEASE);
            }

            CloseHandle(h_target_process);
            return true;
        }

        bool Injector::inject_dll(InjectionMethod inject_method, std::wstring const& dll_path)
        {
            if(Injector::target_process == nullptr) return false;

            switch(inject_method)
            {
                case InjectionMethod::LoadLibraryW:
                    return inject_via_loadlibraryw(dll_path);
                break;

                case InjectionMethod::LdrLoadDll:
                    return inject_via_ldrloaddll(dll_path);
                break;

                case InjectionMethod::SetWindowsHookExW:
                    return inject_via_setwindowshookex(dll_path);
                break;

                case InjectionMethod::ThreadHijacking:
                    return inject_via_thread_hijacking(dll_path, 1000);
                break;
            }

            return false;
        }
    }
}
