#include "pipe.h"

namespace EB
{
    namespace Windows
    {
        namespace Pipe
        {
            // NamedPipeServer
            DWORD WINAPI NamedPipeServer::client_handler(LPVOID lp_param)
            {
                if(lp_param == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_PARAM_NULL_ERROR;

                PipeData* pipe_data = (PipeData*)lp_param;

                if(pipe_data->data_handler == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_DATAHANDLER_NULL_ERROR;

                HANDLE h_heap        = GetProcessHeap();
                BYTE* request_buffer = (BYTE*)HeapAlloc(h_heap, 0, pipe_data->buffer_size);
                BYTE* reply_buffer   = (BYTE*)HeapAlloc(h_heap, 0, pipe_data->buffer_size);

                if(request_buffer == NULL
                || reply_buffer   == NULL)
                    return NAMEDPIPE_CLIENTHANDLER_BUFFER_NULL_ERROR;

                DWORD bytes_read    = 0;
                DWORD reply_bytes   = 0;
                DWORD written_bytes = 0;
                BOOL is_success     = FALSE;

                while(1)
                {
                    is_success = ReadFile(pipe_data->pipe,
                                          request_buffer,
                                          pipe_data->buffer_size,
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

                    pipe_data->data_handler(request_buffer, bytes_read,
                                            reply_buffer,   reply_bytes);

                    if(reply_bytes > 0)
                    {
                        is_success = WriteFile(pipe_data->pipe, 
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
                }

                FlushFileBuffers(pipe_data->pipe);
                DisconnectNamedPipe(pipe_data->pipe);
                CloseHandle(pipe_data->pipe);

                HeapFree(h_heap, 0, request_buffer);
                HeapFree(h_heap, 0, reply_buffer);

                delete pipe_data;

                return NAMEDPIPE_CLIENTHANDLER_ERROR_NONE;
            }

            NamedPipeServer::NamedPipeServer(std::string name)
                : name(name)
            {
            }

            void NamedPipeServer::create_server()
            {
                if(this->data_handler == NULL)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::DataHandler_IS_NULL;
                    return;
                }

                std::string pipe_name = std::string("\\\\.\\pipe\\")+this->name;
                this->is_started = true;

                while(1)
                {
                    HANDLE pipe = CreateNamedPipeA(pipe_name.c_str(),
                                                   PIPE_ACCESS_DUPLEX,
                                                   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                                   PIPE_UNLIMITED_INSTANCES,
                                                   this->buffer_size,
                                                   this->buffer_size,
                                                   0, // client timeout
                                                   NULL);

                    if(pipe == INVALID_HANDLE_VALUE)
                    {
                        this->last_error = NamedPipeServer::LAST_ERROR::CreateNamedPipe_FAILED;
                        return;
                    }

                    BOOL is_connected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

                    this->last_error = LAST_ERROR::NONE;

                    if(is_connected)
                    {
                        // client is connected
                        DWORD thread_id = NULL;

                        PipeData* pipe_data = new PipeData;
                        ZeroMemory(pipe_data, sizeof(PipeData));

                        pipe_data->pipe         = pipe;
                        pipe_data->buffer_size  = this->buffer_size;
                        pipe_data->data_handler = this->data_handler;

                        HANDLE h_thread = CreateThread(NULL,
                                                       0,
                                                       NamedPipeServer::client_handler,
                                                       (LPVOID)pipe_data,
                                                       0,
                                                       &thread_id);

                        if(h_thread == NULL)
                        {
                            delete pipe_data;

                            this->last_error = NamedPipeServer::LAST_ERROR::CreateThread_FAILED;
                            return;
                        }
                        else CloseHandle(h_thread);
                    }
                    else CloseHandle(pipe);
                }

                this->is_started = false;
            }

            void NamedPipeServer::set_buffer_size(size_t const& size)
            {
                if(this->is_started)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::SERVER_IS_RUNNING;
                    return;
                }

                this->buffer_size = size;
                this->last_error  = LAST_ERROR::NONE;
            }

            void NamedPipeServer::set_data_handler(DataHandler func)
            {
                if(this->is_started)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::SERVER_IS_RUNNING;
                    return;
                }

                this->data_handler = func;
                this->last_error  = LAST_ERROR::NONE;
            }

            void NamedPipeServer::set_data_handler(DataHandler& func)
            {
                if(this->is_started)
                {
                    this->last_error = NamedPipeServer::LAST_ERROR::SERVER_IS_RUNNING;
                    return;
                }

                this->data_handler = func;
                this->last_error  = LAST_ERROR::NONE;
            }

            NamedPipeServer::LAST_ERROR NamedPipeServer::get_last_error() const
            {
                return this->last_error;
            }

            // NamedPipeClient
            NamedPipeClient::NamedPipeClient(std::string name, size_t recieve_buffer_size, size_t timeout)
                : name(name), buffer_size(recieve_buffer_size)
            {
                std::string pipe_name = std::string("\\\\.\\pipe\\")+this->name;

                while(1)
                {
                    this->pipe = CreateFileA(pipe_name.c_str(),
                                             GENERIC_READ | GENERIC_WRITE,
                                             0,
                                             NULL,
                                             OPEN_EXISTING,
                                             0,
                                             NULL);

                    if(this->pipe != INVALID_HANDLE_VALUE) break;

                    if(GetLastError() != ERROR_PIPE_BUSY)
                    {
                        this->last_error = LAST_ERROR::CANT_OPEN_PIPE;
                        return;
                    }

                    if(!WaitNamedPipeA(pipe_name.c_str(), timeout))
                    {
                        this->last_error = LAST_ERROR::WaitNamedPipe_TIMEOUTED;
                        return;
                    }
                }

                // Pipe connected
                BOOL  is_success = FALSE;
                DWORD mode       = PIPE_READMODE_BYTE;

                is_success = SetNamedPipeHandleState(this->pipe, &mode, NULL, NULL);

                if(!is_success)
                {
                    this->last_error = LAST_ERROR::SetNamedPipeHandleState_FAILED;
                    return;
                }

                this->buffer       = new BYTE[this->buffer_size];
                this->last_error   = LAST_ERROR::NONE;
                this->is_pipe_open = true;
            }

            NamedPipeClient::~NamedPipeClient()
            {
                delete[] this->buffer;
            }

            bool NamedPipeClient::write(BYTE* bytes, size_t const& size)
            {
                if(this->pipe == NULL)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NULL;
                    return false;
                }
                else if(!this->is_pipe_open)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NOT_OPEN;
                    return false;
                }

                DWORD written_bytes = 0;
                BOOL is_success = WriteFile(this->pipe,
                                            bytes,
                                            size,
                                            &written_bytes,
                                            NULL);

                if(!is_success)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::WriteFile_FAILED;
                    return false;
                }
                else if(written_bytes != size)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::NOT_ALL_BYTES_WRITTEN;
                    return false;
                }

                this->last_error = NamedPipeClient::LAST_ERROR::NONE;
                return true;
            }

            bool NamedPipeClient::write(std::vector<BYTE> const& bytes)
            {
                if(this->pipe == NULL)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NULL;
                    return false;
                }
                else if(!this->is_pipe_open)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NOT_OPEN;
                    return false;
                }

                DWORD written_bytes = 0;
                BOOL is_success = WriteFile(this->pipe,
                                            bytes.data(),
                                            bytes.size(),
                                            &written_bytes,
                                            NULL);

                if(!is_success)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::WriteFile_FAILED;
                    return false;
                }
                else if(written_bytes != bytes.size())
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::NOT_ALL_BYTES_WRITTEN;
                    return false;
                }

                this->last_error = NamedPipeClient::LAST_ERROR::NONE;
                return true;
            }

            size_t NamedPipeClient::read()
            {
                if(this->pipe == NULL)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NULL;
                    return false;
                }
                else if(!this->is_pipe_open)
                {
                    this->last_error = NamedPipeClient::LAST_ERROR::PIPE_IS_NOT_OPEN;
                    return false;
                }

                DWORD bytes_read = 0;
                BOOL  is_success = FALSE;

                do
                {
                    is_success = ReadFile(this->pipe,
                                          this->buffer,
                                          this->buffer_size,
                                          &bytes_read,
                                          NULL);

                    if(!is_success && GetLastError() != ERROR_MORE_DATA)
                        break; 
                }
                while(!is_success);

                if(is_success) this->last_error = NamedPipeClient::LAST_ERROR::NONE;

                return bytes_read;
            }

            BYTE const* NamedPipeClient::get_buffer() const
            {
                return this->buffer;
            }

            NamedPipeClient::LAST_ERROR NamedPipeClient::get_last_error() const
            {
                return this->last_error;
            }
        }
    }
}
