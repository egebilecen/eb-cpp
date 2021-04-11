#pragma once

#include <Windows.h>
#include <tlhelp32.h>

#include <string>
#include <vector>

#include "shellcode.h"
#include "../string.h"

namespace EB
{
    namespace Process
    {
        /* Definations for NtQuerySystemInformation */
        typedef NTSTATUS (WINAPI* _NtQuerySystemInformation)(int, PVOID, ULONG, PULONG);

        typedef LONG KPRIORITY;

        struct CLIENT_ID
        {
            DWORD UniqueProcess; // Process ID
            #ifdef _WIN64
            ULONG pad1;
            #endif
            DWORD UniqueThread;  // Thread ID
            #ifdef _WIN64
            ULONG pad2;
            #endif
        };

        typedef struct
        {
            FILETIME ProcessorTime;
            FILETIME UserTime;
            FILETIME CreateTime;
            ULONG WaitTime;
            #ifdef _WIN64
            ULONG pad1;
            #endif
            PVOID StartAddress;
            CLIENT_ID Client_Id;
            KPRIORITY CurrentPriority;
            KPRIORITY BasePriority;
            ULONG ContextSwitchesPerSec;
            ULONG ThreadState;
            ULONG ThreadWaitReason;
            ULONG pad2;
        } SYSTEM_THREAD_INFORMATION;

        typedef struct
        {
            USHORT Length;
            USHORT MaximumLength;
            PWSTR  Buffer;
        } UNICODE_STRING;

        typedef struct
        {
            ULONG_PTR PeakVirtualSize;
            ULONG_PTR VirtualSize;
            ULONG PageFaultCount;
            #ifdef _WIN64
            ULONG pad1;
            #endif
            ULONG_PTR PeakWorkingSetSize;
            ULONG_PTR WorkingSetSize;
            ULONG_PTR QuotaPeakPagedPoolUsage;
            ULONG_PTR QuotaPagedPoolUsage;
            ULONG_PTR QuotaPeakNonPagedPoolUsage;
            ULONG_PTR QuotaNonPagedPoolUsage;
            ULONG_PTR PagefileUsage;
            ULONG_PTR PeakPagefileUsage;
        } VM_COUNTERS;

        typedef struct
        {
            ULONG NextOffset;
            ULONG ThreadCount;
            LARGE_INTEGER WorkingSetPrivateSize;
            ULONG HardFaultCount;
            ULONG NumberOfThreadsHighWatermark;
            ULONGLONG CycleTime;
            FILETIME CreateTime;
            FILETIME UserTime;
            FILETIME KernelTime;
            UNICODE_STRING ImageName;
            KPRIORITY BasePriority;
            #ifdef _WIN64
            ULONG pad1;
            #endif
            ULONG ProcessId;
            #ifdef _WIN64
            ULONG pad2;
            #endif
            ULONG InheritedFromProcessId;
            #ifdef _WIN64
            ULONG pad3;
            #endif
            ULONG HandleCount;
            ULONG SessionId;
            ULONG_PTR UniqueProcessKey; // always NULL, use SystemExtendedProcessInformation (57) to get value
            VM_COUNTERS VirtualMemoryCounters;
            ULONG_PTR PrivatePageCount;
            IO_COUNTERS IoCounters;
            SYSTEM_THREAD_INFORMATION ThreadInfos[1];
        } SYSTEM_PROCESS_INFORMATION;

        #define SYSTEMPROCESSINFORMATION 5
        #define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xC0000004)
        /* End Definations for NtQuerySystemInformation */

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

        enum class ThreadEnumerationMethod
        {
            CreateToolhelp32Snapshot,
            NtQuerySystemInformation
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
            bool load_thread_list(ThreadEnumerationMethod method=ThreadEnumerationMethod::CreateToolhelp32Snapshot);
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
