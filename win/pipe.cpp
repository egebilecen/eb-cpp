#include "pipe.h"

namespace EB
{
    namespace Windows
    {
        namespace Pipe
        {
            DWORD WINAPI NamedPipeServer::client_handler(LPVOID lp_param)
            {
                if(lp_param == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_PARAM_NULL_ERROR;

                NamedPipeServer* named_pipe_server = (NamedPipeServer*) lp_param;

                if(named_pipe_server->data_handler == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_DATAHANDLER_NULL_ERROR;

                HANDLE h_heap        = GetProcessHeap();
                BYTE* request_buffer = (BYTE*)HeapAlloc(h_heap, 0, named_pipe_server->buffer_size);
                BYTE* reply_buffer   = (BYTE*)HeapAlloc(h_heap, 0, named_pipe_server->buffer_size);

                if(request_buffer == NULL
                   || reply_buffer   == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_BUFFER_NULL_ERROR;

                DWORD bytes_read    = 0;
                DWORD reply_bytes   = 0;
                DWORD written_bytes = 0;
                BOOL is_success     = FALSE;

                while(1)
                {
                    is_success = ReadFile(named_pipe_server->pipe,
                                          request_buffer,
                                          named_pipe_server->buffer_size,
                                          &bytes_read,
                                          NULL);

                    if(!is_success || bytes_read == 0)
                    {
                        if(GetLastError() == ERROR_BROKEN_PIPE)
                        {
                            // client disconnected
                        }
                        else
                        {
                            // ReadFile failed
                        }

                        break;
                    }

                    named_pipe_server->data_handler(request_buffer, bytes_read,
                                                    reply_buffer,   reply_bytes);

                    is_success = WriteFile(named_pipe_server->pipe, 
                                           reply_buffer, 
                                           reply_bytes,
                                           &written_bytes, 
                                           NULL);

                    if(!is_success || reply_bytes != written_bytes)
                    {
                        // WriteFile failed
                        break;
                    }
                }

                FlushFileBuffers(named_pipe_server->pipe);
                DisconnectNamedPipe(named_pipe_server->pipe);
                CloseHandle(named_pipe_server->pipe);

                HeapFree(h_heap, 0, request_buffer);
                HeapFree(h_heap, 0, reply_buffer);

                return NAMEDPIPE_CLIENTHANDLER_ERROR_NONE;
            }

            NamedPipeServer::NamedPipeServer(std::string name)
                : name(name)
            {
            }

            void NamedPipeServer::create_server(HANDLE const* pipe_out)
            {
                if(this->pipe != NULL) 
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::PIPE_IS_NOT_NULL;
                    return;
                }
                else if(this->data_handler == NULL)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::DataHandler_IS_NULL;
                    return;
                }

                while(1)
                {
                    this->pipe = CreateNamedPipeA((std::string("\\\\.\\pipe\\")+this->name).c_str(),
                                                  PIPE_ACCESS_DUPLEX,
                                                  PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                                  PIPE_UNLIMITED_INSTANCES,
                                                  this->buffer_size,
                                                  this->buffer_size,
                                                  0, // client timeout
                                                  NULL);

                    if(this->pipe == INVALID_HANDLE_VALUE)
                    {
                        this->last_error = NamedPipeServer::LAST_ERROR::CreateNamedPipe_FAILED;
                        return;
                    }

                    pipe_out = &this->pipe;

                    BOOL is_connected = ConnectNamedPipe(this->pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

                    this->last_error = LAST_ERROR::NONE;

                    if(is_connected)
                    {
                        // client is connected
                        DWORD thread_id = NULL;

                        HANDLE h_thread = CreateThread(NULL,
                                                       0,
                                                       NamedPipeServer::client_handler,
                                                       (LPVOID)this,
                                                       0,
                                                       &thread_id);

                        if(h_thread == NULL)
                        {
                            this->last_error = NamedPipeServer::LAST_ERROR::CreateThread_FAILED;
                            return;
                        }
                        else CloseHandle(h_thread);
                    }
                    else
                    {
                        CloseHandle(this->pipe);
                        this->pipe = NULL;
                    }
                }
            }

            void NamedPipeServer::set_buffer_size(size_t const& size)
            {
                if(this->pipe != NULL)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::PIPE_IS_NOT_NULL;
                    return;
                }

                this->buffer_size = size;
                this->last_error  = LAST_ERROR::NONE;
            }

            void NamedPipeServer::set_data_handler(DataHandler func)
            {
                this->data_handler = func;
            }

            void NamedPipeServer::set_data_handler(DataHandler& func)
            {
                this->data_handler = func;
            }

            NamedPipeServer::LAST_ERROR NamedPipeServer::get_last_error() const
            {
                return this->last_error;
            }
        }
    }
}