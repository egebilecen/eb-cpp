#include "process.h"

namespace EB
{
    // Private Method(s)
    void Process::get_process_info(DWORD const* process_id, std::string const* process_name)
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

        if(this->process_info != nullptr)
            this->process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *this->get_process_id());

        return;
    }

    // Constructor(s)
    Process::Process(std::string const& process_name)
    {
        this->get_process_info(nullptr, &process_name);
    }

    Process::Process(DWORD const& process_id)
    {
        this->get_process_info(&process_id, nullptr);
    }

    // Destructor
    Process::~Process()
    {
        delete this->process_info;

        if(this->process_handle != NULL)
            CloseHandle(this->process_handle);
    }

    // Public Method(s)
    PROCESSENTRY32 const* Process::get_process_info() const
    {
        return this->process_info;
    }

    DWORD const* Process::get_process_id() const
    {
        return this->process_info != nullptr ? &this->process_info->th32ProcessID : nullptr;
    }

    HANDLE const* Process::get_handle() const
    {
        return this->process_info != nullptr ? &this->process_handle : nullptr;
    }

    bool Process::load_module_list()
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

    loop_end:
        CloseHandle(modules_snapshot);

        return true;
    }

    std::vector<Process::ModuleInfo> const* Process::get_module_list() const
    {
        return &this->module_list;
    }
}
