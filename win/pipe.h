#pragma once

#include <Windows.h>
#include <string>
#include <vector>

// DEBUG
#define NAMEDPIPE_DEBUG 1
#if NAMEDPIPE_DEBUG
#define PRINT_LASTERROR() do{ std::cout << GetLastError() << "\n"; }while(0);
#include <iostream>
#endif
// end DEBUG

#define NAMEDPIPE_CLIENTHANDLER_ERROR_NONE             1
#define NAMEDPIPE_CLIENTHANDLER_PARAM_NULL_ERROR       ((DWORD)-1)
#define NAMEDPIPE_CLIENTHANDLER_BUFFER_NULL_ERROR      ((DWORD)-2)
#define NAMEDPIPE_CLIENTHANDLER_DATAHANDLER_NULL_ERROR ((DWORD)-3)

namespace EB
{
    namespace Windows
    {
        namespace Pipe
        {
            class NamedPipeServer
            {
            public:
                typedef void(*DataHandler)(BYTE* request_buffer, DWORD const& request_size, 
                                           BYTE* reply_buffer,   DWORD&       reply_size);

                enum LAST_ERROR
                {
                    NONE,
                    CreateNamedPipe_FAILED,
                    CreateThread_FAILED,
                    DataHandler_IS_NULL,
                    SERVER_IS_RUNNING
                };

                struct PipeData
                {
                    HANDLE pipe;
                    size_t buffer_size;
                    DataHandler data_handler;
                };

            private:
                std::string  name;
                size_t       buffer_size  = 256;
                DataHandler  data_handler = NULL;
                bool         is_started   = false;
                LAST_ERROR   last_error   = LAST_ERROR::NONE;

                static DWORD WINAPI client_handler(LPVOID lp_param);

            public:
                NamedPipeServer(std::string name);
                void create_server();
                void set_buffer_size(size_t const& size);
                void set_data_handler(DataHandler  func);
                void set_data_handler(DataHandler& func);
                LAST_ERROR get_last_error() const;
            };

            class NamedPipeClient
            {
            public:
                typedef void(*DataHandler)(BYTE* request_buffer, DWORD const& request_size, 
                                           BYTE* reply_buffer,   DWORD&       reply_size);

                enum LAST_ERROR
                {
                    NONE,
                    PIPE_IS_NULL,
                    CreateFile_FAILED,
                    DataHandler_IS_NULL,
                    CANT_OPEN_PIPE,
                    WaitNamedPipe_TIMEOUTED,
                    SetNamedPipeHandleState_FAILED,
                    NOT_ALL_BYTES_WRITTEN,
                    WriteFile_FAILED,
                    PIPE_IS_NOT_OPEN
                };

            private:
                std::string  name;
                BYTE*        buffer       = NULL;
                size_t       buffer_size;

                HANDLE       pipe         = NULL;
                bool         is_pipe_open = false;
                LAST_ERROR   last_error   = LAST_ERROR::NONE;
                DataHandler  data_handler = NULL;

            public:
                NamedPipeClient(std::string name, size_t recieve_buffer_size=256, size_t timeout_ms=10000);
                ~NamedPipeClient();
                bool write(BYTE* bytes, size_t const& size);
                bool write(std::vector<BYTE> const& bytes);
                size_t read();
                BYTE const* get_buffer() const;
                LAST_ERROR get_last_error() const;
            };
        }
    }
}
