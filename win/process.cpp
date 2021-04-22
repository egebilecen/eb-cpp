#include "process.h"

namespace EB
{
    namespace Windows
    {
        namespace Process
        {
            void loop_process_list(LoopProcessListCallback callback)
            {
                PROCESSENTRY32W* process_info = new PROCESSENTRY32W();
                process_info->dwSize = sizeof(*process_info);

                HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

                if(processes_snapshot == INVALID_HANDLE_VALUE) return;

                if(Process32FirstW(processes_snapshot, process_info))
                {
                    do
                    {
                        if(!callback(process_info))
                            goto loop_end;
                    } 
                    while(Process32NextW(processes_snapshot, process_info));
                }

            loop_end:
                CloseHandle(processes_snapshot);
                return;
            }

            /* External Process */
            // Private Method(s)
            void ExternalProcess::get_process_info(DWORD const* process_id, std::wstring const* process_name)
            {
                if(process_id    == nullptr
                && process_name  == nullptr) return;

                PROCESSENTRY32W* process_info = nullptr;

                loop_process_list([&process_info, &process_id, &process_name](PROCESSENTRY32W* proc_info) -> bool
                {
                    if(process_id != nullptr)
                    {
                        if(*process_id == proc_info->th32ProcessID)
                        {
                            process_info = proc_info;
                            return false;
                        }
                    }
                    else if(process_name != nullptr)
                    {
                        if(!process_name->compare(proc_info->szExeFile))
                        {
                            process_info = proc_info;
                            return false;
                        }
                    }

                    return true;
                });

                this->process_info = process_info;
            }

            // Constructor(s)
            ExternalProcess::ExternalProcess(std::wstring const& process_name)
            {
                this->get_process_info(nullptr, &process_name);
            }

            ExternalProcess::ExternalProcess(DWORD const& process_id)
            {
                this->get_process_info(&process_id, nullptr);
            }

            // Destructor
            ExternalProcess::~ExternalProcess()
            {
                delete this->process_info;

                if(this->process_handle != NULL)
                    CloseHandle(this->process_handle);
            }

            // Public Method(s)
            PROCESSENTRY32W const* ExternalProcess::get_process_info() const
            {
                return this->process_info;
            }

            DWORD const* ExternalProcess::get_process_id() const
            {
                return this->process_info != nullptr ? &this->process_info->th32ProcessID : nullptr;
            }

            bool ExternalProcess::load_module_list()
            {
                if(this->process_info == nullptr) return false;

                MODULEENTRY32W module_info;
                module_info.dwSize = sizeof(module_info);

                HANDLE modules_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, *this->get_process_id());

                if(modules_snapshot == INVALID_HANDLE_VALUE) return false;

                this->module_list.clear();

                if(Module32FirstW(modules_snapshot, &module_info))
                {
                    do
                    {
                        ModuleInfo mod_info(std::wstring(module_info.szExePath), std::wstring(module_info.szModule),
                                            module_info.modBaseAddr,             module_info.modBaseSize,
                                            module_info.hModule);

                        this->module_list.push_back(mod_info);
                    } 
                    while(Module32NextW(modules_snapshot, &module_info));
                }

                CloseHandle(modules_snapshot);
                SetLastError(NO_ERROR);
                return true;
            }

            bool ExternalProcess::load_thread_list(ThreadEnumerationMethod method)
            {
                if(this->process_info == nullptr) return false;

                switch(method)
                {
                    case ThreadEnumerationMethod::CreateToolhelp32Snapshot:
                    {
                        THREADENTRY32 thread_info;
                        thread_info.dwSize = sizeof(thread_info);

                        HANDLE thread_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);

                        if(thread_snapshot == INVALID_HANDLE_VALUE) return false;

                        this->thread_list.clear();

                        if(Thread32First(thread_snapshot, &thread_info))
                        {
                            do
                            {
                                if(thread_info.th32OwnerProcessID == *this->get_process_id())
                                {
                                    ThreadInfo th_info(thread_info.th32ThreadID, thread_info.th32OwnerProcessID, thread_info.tpBasePri);
                                    this->thread_list.push_back(th_info);
                                }
                            } 
                            while(Thread32Next(thread_snapshot, &thread_info));
                        }

                        CloseHandle(thread_snapshot);
                    }
                    break;

                    case ThreadEnumerationMethod::NtQuerySystemInformation:
                    {
                        LPVOID lp_ntquerysysteminformation = GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");

                        if(!lp_ntquerysysteminformation) return false;

                        SYSTEM_PROCESS_INFORMATION* proc_info = NULL;
                        BYTE* buffer = NULL;
                        ULONG buffer_len = 0;

                        NTSTATUS result = ((_NtQuerySystemInformation)(lp_ntquerysysteminformation))(SYSTEMPROCESSINFORMATION,
                                                                                                     buffer, buffer_len, &buffer_len);

                        if(result == (NTSTATUS)0xC0000004) // STATUS_INFO_LENGTH_MISMATCH
                            buffer = (BYTE*)LocalAlloc(LMEM_FIXED, buffer_len);

                        ((_NtQuerySystemInformation)(lp_ntquerysysteminformation))(SYSTEMPROCESSINFORMATION,
                                                                                   buffer, buffer_len, &buffer_len);

                        unsigned int i = 0;

                        do
                        {
                            proc_info = (SYSTEM_PROCESS_INFORMATION*)&buffer[i];

                            if(proc_info->ProcessId == *this->get_process_id())
                            {
                                for(int j=0; j < proc_info->ThreadCount; j++)
                                {
                                    ThreadInfo th_info(proc_info->ThreadInfos[j].Client_Id.UniqueThread, proc_info->ProcessId, proc_info->ThreadInfos[j].BasePriority);
                                    this->thread_list.push_back(th_info);
                                }
                            }

                            i += proc_info->NextOffset;
                        }
                        while(proc_info->NextOffset != 0 && proc_info != NULL);

                        LocalFree(buffer);
                    }
                    break;
                }

                SetLastError(NO_ERROR);
                return true;
            }

            std::vector<Process::ModuleInfo> const* ExternalProcess::get_module_list() const
            {
                return &this->module_list;
            }

            std::vector<ThreadInfo> const* ExternalProcess::get_thread_list() const
            {
                return &this->thread_list;
            }

            HANDLE ExternalProcess::get_process_handle() const
            {
                return OpenProcess(PROCESS_ALL_ACCESS, FALSE, *this->get_process_id());
            }

            ModuleInfo const* ExternalProcess::get_module(std::wstring const& module_name) const
            {
                for(size_t i=0; i < this->module_list.size(); i++)
                {
                    if(this->module_list[i].module_name == module_name)
                        return &this->module_list[i];
                }

                return nullptr;
            }

            /* Internal Process */
            namespace InternalProcess
            {
                DWORD get_process_id()
                {
                    return GetCurrentProcessId();
                }

                HANDLE get_process_handle()
                {
                    return GetCurrentProcess();
                }

                HMODULE get_module_handle()
                {
                    return GetModuleHandleW(NULL);
                }

                std::vector<ModuleInfo> get_module_list()
                {
                    std::vector<ModuleInfo> mod_list;

                    MODULEENTRY32W module_info;
                    module_info.dwSize = sizeof(module_info);

                    HANDLE modules_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, get_process_id());

                    if(modules_snapshot == INVALID_HANDLE_VALUE) return mod_list;

                    if(Module32FirstW(modules_snapshot, &module_info))
                    {
                        do
                        {
                            ModuleInfo mod_info(std::wstring(module_info.szExePath), std::wstring(module_info.szModule),
                                                module_info.modBaseAddr,             module_info.modBaseSize,
                                                module_info.hModule);

                            mod_list.push_back(mod_info);
                        } 
                        while(Module32NextW(modules_snapshot, &module_info));
                    }

                    CloseHandle(modules_snapshot);
                    SetLastError(NO_ERROR);
                    return mod_list;
                }

                ModuleInfo const* get_module(std::vector<ModuleInfo> const* module_list, std::wstring const& module_name)
                {
                    for(size_t i=0; i < module_list->size(); i++)
                    {
                        if((*module_list)[i].module_name == module_name)
                            return &(*module_list)[i];
                    }

                    return nullptr;
                }
            }

            /* Injector */
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
}
