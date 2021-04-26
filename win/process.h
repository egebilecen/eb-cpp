#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <strsafe.h>

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>

#include "shellcode.h"

namespace EB
{
    namespace Windows
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
            #define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
            /* End Definations for NtQuerySystemInformation */

            /* Definations for NtCreateThreadEx */
            typedef NTSTATUS (NTAPI* _NtCreateThreadEx)(OUT PHANDLE                ThreadHandle,
                                                        IN  ACCESS_MASK            DesiredAccess,
                                                        IN  LPVOID                 ObjectAttributes,
                                                        IN  HANDLE                 ProcessHandle,
                                                        IN  LPTHREAD_START_ROUTINE ThreadProcedure,
                                                        IN  LPVOID                 ParameterData,
                                                        IN  BOOL                   CreateSuspended,
                                                        IN  SIZE_T                 StackZeroBits,
                                                        IN  SIZE_T                 SizeOfStackCommit,
                                                        IN  SIZE_T                 SizeOfStackReserve,
                                                        OUT LPVOID                 BytesBuffer);
            /* End Definations for NtCreateThreadEx */

            /* Definations for RtlCreateUserThread */
            typedef NTSTATUS(NTAPI* _RtlCreateUserThread)(IN HANDLE               ProcessHandle,
                                                          IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
                                                          IN BOOLEAN              CreateSuspended,
                                                          IN ULONG                StackZeroBits,
                                                          IN OUT PULONG           StackReserved,
                                                          IN OUT PULONG           StackCommit,
                                                          IN PVOID                StartAddress,
                                                          IN PVOID                StartParameter OPTIONAL,
                                                          OUT PHANDLE             ThreadHandle,
                                                          OUT CLIENT_ID*          ClientID);
            /* End Definations for RtlCreateUserThread */

            // Type definations
            typedef std::function<bool(PROCESSENTRY32W*)> LoopProcessListCallback;

            // Data Types
            struct EnumData
            {
                DWORD process_id;
                HWND  h_wnd;
            };

            struct ModuleInfo
            {
                std::wstring module_path;      // szExePath
                std::wstring module_name;      // szModule
                uintptr_t    module_base_addr; // modBaseAddr
                DWORD        module_base_size; // modBaseSize
                HMODULE      module_handle;    // hModule

                ModuleInfo(std::wstring module_path,      std::wstring module_name,
                           uintptr_t    module_base_addr, DWORD        module_base_size,
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

            enum class ThreadCreationMethod
            {
                CreateRemoteThread,
                NtCreateThreadEx,
                RtlCreateUserThread
            };

            // Private Functions
            BOOL CALLBACK _enum_proc(HWND hWnd, LPARAM lParam);

            // Public Functions
            void loop_process_list(LoopProcessListCallback callback);
            HWND get_window_from_process_id(DWORD const& process_id);
            HWND get_window_from_process_handle(HANDLE const& handle);

            // External Process Class
            class ExternalProcess
            {
            private:
                // Variables
                PROCESSENTRY32W* process_info   = nullptr;
                HANDLE           process_handle = NULL;
                std::vector<ModuleInfo> module_list;
                std::vector<ThreadInfo> thread_list;

                // Method(s)
                void get_process_info(DWORD const* process_id, std::wstring const* process_name);

            public:
                // Constructor(s)
                ExternalProcess(std::wstring const& process_name);
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
                std::vector<ThreadInfo> const* get_thread_list() const;
                ModuleInfo const* get_module(std::wstring const& module_name) const;
            };

            // Internal Process Namespace
            namespace InternalProcess
            {
                DWORD get_process_id();
                HANDLE get_process_handle();
                HMODULE get_module_handle();
                std::vector<ModuleInfo> get_module_list();
                ModuleInfo const* get_module(std::vector<ModuleInfo> const* module_list, std::wstring const& module_name);
            }
        }
    }
}
