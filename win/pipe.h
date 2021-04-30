#pragma once

#include <Windows.h>
#include <string>

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

                enum class LAST_ERROR
                {
                    NONE,
                    PIPE_IS_NOT_NULL,
                    CreateNamedPipe_FAILED,
                    CreateThread_FAILED,
                    DataHandler_IS_NULL
                };

            private:
                std::string  name;
                HANDLE       pipe         = NULL;
                size_t       buffer_size  = 256;
                LAST_ERROR   last_error   = LAST_ERROR::NONE;
                DataHandler  data_handler = NULL;

                static DWORD WINAPI client_handler(LPVOID lp_param);

            public:
                NamedPipeServer(std::string name);
                void create_server(HANDLE const* pipe_out);
                void set_buffer_size(size_t const& size);
                void set_data_handler(DataHandler  func);
                void set_data_handler(DataHandler& func);
                LAST_ERROR get_last_error() const;
            };
        }
    }
}