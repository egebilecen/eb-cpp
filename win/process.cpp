#include "process.h"

namespace EB
{
    namespace Windows
    {
        namespace Process
        {
            // Private Functions
            BOOL CALLBACK _enum_proc(HWND hWnd, LPARAM lParam)
            {
                EnumData& ed = *(EnumData*)lParam;
                DWORD process_id = NULL;

                GetWindowThreadProcessId(hWnd, &process_id);

                if(ed.process_id == process_id)
                {
                    ed.h_wnd = hWnd;
                    SetLastError(ERROR_SUCCESS);

                    return FALSE;
                }

                return TRUE;
            }

            // Public Functions
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

            HWND get_window_from_process_id(DWORD const& process_id)
            {
                EnumData ed = { process_id, NULL };

                if(!EnumWindows(_enum_proc, (LPARAM)&ed)
                && GetLastError() == ERROR_SUCCESS) return ed.h_wnd;

                return NULL;
            }

            HWND get_window_from_process_handle(HANDLE const& handle)
            {
                return get_window_from_process_id(GetProcessId(handle));
            }

            // External Process Class
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

            // Internal Process Namespace
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
        }
    }
}
