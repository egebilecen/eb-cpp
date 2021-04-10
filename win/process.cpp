#include "process.h"

namespace EB
{
    namespace Process
    {
        // Private Method(s)
        void ExternalProcess::get_process_info(DWORD const* process_id, std::string const* process_name)
        {
            if(process_id   == nullptr
            && process_name == nullptr) return;

            PROCESSENTRY32* process_info = new PROCESSENTRY32();
            process_info->dwSize = sizeof(*process_info);

            HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

            if(processes_snapshot == INVALID_HANDLE_VALUE) return;

            if(Process32First(processes_snapshot, process_info))
            {
                do
                {
                    if(process_id != nullptr)
                    {
                        if(*process_id == process_info->th32ProcessID)
                        {
                            this->process_info = process_info;
                            goto loop_end;
                        }
                    }
                    else if(process_name != nullptr)
                    {
                        std::wstring process_name_w = String::to_wstring(*process_name);

                        if(!process_name_w.compare(process_info->szExeFile))
                        {
                            this->process_info = process_info;
                            goto loop_end;
                        }
                    }
                } 
                while(Process32Next(processes_snapshot, process_info));
            }

        loop_end:
            CloseHandle(processes_snapshot);
            return;
        }

        // Constructor(s)
        ExternalProcess::ExternalProcess(std::string const& process_name)
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
        PROCESSENTRY32 const* ExternalProcess::get_process_info() const
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

            MODULEENTRY32 module_info;
            module_info.dwSize = sizeof(module_info);

            HANDLE modules_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, *this->get_process_id());

            if(modules_snapshot == INVALID_HANDLE_VALUE) return false;

            this->module_list.clear();

            if(Module32First(modules_snapshot, &module_info))
            {
                do
                {
                    ModuleInfo mod_info(std::wstring(module_info.szExePath), std::wstring(module_info.szModule),
                                        module_info.modBaseAddr,             module_info.modBaseSize,
                                        module_info.hModule);

                    this->module_list.push_back(mod_info);
                } 
                while(Module32Next(modules_snapshot, &module_info));
            }

            CloseHandle(modules_snapshot);
            return true;
        }

        std::vector<Process::ModuleInfo> const* ExternalProcess::get_module_list() const
        {
            return &this->module_list;
        }

        HANDLE ExternalProcess::get_process_handle() const
        {
            return OpenProcess(PROCESS_ALL_ACCESS, FALSE, *this->get_process_id());
        }

        ModuleInfo const* ExternalProcess::get_module(std::string const& module_name) const
        {
            for(size_t i=0; i < this->module_list.size(); i++)
            {
                if(this->module_list[i].module_name == String::to_wstring(module_name))
                    return &this->module_list[i];
            }

            return nullptr;
        }

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
                return GetModuleHandle(NULL);
            }

            std::vector<ModuleInfo> get_module_list()
            {
                std::vector<ModuleInfo> mod_list;

                MODULEENTRY32 module_info;
                module_info.dwSize = sizeof(module_info);

                HANDLE modules_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, get_process_id());

                if(modules_snapshot == INVALID_HANDLE_VALUE) return mod_list;

                if(Module32First(modules_snapshot, &module_info))
                {
                    do
                    {
                        ModuleInfo mod_info(std::wstring(module_info.szExePath), std::wstring(module_info.szModule),
                                            module_info.modBaseAddr,             module_info.modBaseSize,
                                            module_info.hModule);

                        mod_list.push_back(mod_info);
                    } 
                    while(Module32Next(modules_snapshot, &module_info));
                }

                CloseHandle(modules_snapshot);
                return mod_list;
            }

            ModuleInfo const* get_module(std::vector<ModuleInfo> const* module_list, std::string const& module_name)
            {
                for(size_t i=0; i < module_list->size(); i++)
                {
                    if((*module_list)[i].module_name == String::to_wstring(module_name))
                        return &(*module_list)[i];
                }

                return nullptr;
            }
        }

        namespace Injector
        {
            ExternalProcess* ExternalInjector::external_process = nullptr;

            void ExternalInjector::set_target_process(ExternalProcess* external_process)
            {
                ExternalInjector::external_process = external_process;
            }

            bool ExternalInjector::inject_via_loadlibraryw(std::string const& dll_path)
            {
                if(ExternalInjector::external_process == nullptr) return false;

                LPVOID lp_loadlibraryw = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

                // GetProcAddress failed
                if(!lp_loadlibraryw) return false;

                // Get target process' handle
                HANDLE h_target_process = ExternalInjector::external_process->get_process_handle();

                // Failed to get handle of target process
                if(!h_target_process) return false;

                // Allocate dll_path string in target process' memory
                std::wstring dll_path_w = String::to_wstring(dll_path);
                WCHAR dll_path_str[MAX_PATH];
                ZeroMemory(dll_path_str, sizeof(dll_path_str));
                dll_path_w.copy(dll_path_str, MAX_PATH);

                LPVOID lp_dll_path = VirtualAllocEx(h_target_process, NULL, sizeof(dll_path_str), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                
                // Write data to memory
                WriteProcessMemory(h_target_process, lp_dll_path, dll_path_str, sizeof(dll_path_str), NULL);

                // Create thread to execute LoadLibraryW function
                HANDLE h_thread = CreateRemoteThread(h_target_process, NULL, NULL, 
                                                     (LPTHREAD_START_ROUTINE)lp_loadlibraryw, lp_dll_path, 
                                                     NULL, NULL);

                CloseHandle(h_target_process);

                if(!h_thread) return false;

                return true;
            }

            bool ExternalInjector::inject_dll(InjectionMethod inject_method, std::string const& dll_path)
            {
                if(ExternalInjector::external_process == nullptr) return false;

                switch(inject_method)
                {
                    case InjectionMethod::LoadLibraryW:
                        return inject_via_loadlibraryw(dll_path);
                    break;
                }

                return false;
            }
        }
    }
}
