#include "server.h"

namespace EB
{
    namespace TCP
    {
        //--------------------------------- Constructor Defination
        Server::Server(std::string         const* ip,
                       uint16_t            const* port,
                       EB::Protocol::Base *const  protocol,
                       bool                       debug) : protocol(protocol)
        {
            Logger::Log("Initializing the server class.",
                        __FILE__, __LINE__);
            this->ip       = ip;
            this->port     = port;
            this->debug    = debug;
        }

        //---------------- Public
        //-------- Server
        void Server::start()
        {
            //create socket
            Logger::Log("Creating socket.",
                        __FILE__, __LINE__);
            this->server->socket = socket(this->server->settings.family, this->server->settings.type, 0x00);

            if(this->server->socket == -1)
            {
                this->last_error_code = ERROR_CODE::SOCKET_CREATE;
                Logger::Log("Couldn't create socket. (Error code: "+to_string(this->last_error_code)+")",
                            __FILE__, __LINE__,
                            Logger::LOG_TYPE::ERROR);
                return;
            }

            //bind
            Logger::Log("Binding the socket.",
                        __FILE__, __LINE__);
            this->server->hint.sin_family = this->server->settings.family;
            this->server->hint.sin_port   = htons(*this->port);
            inet_pton(this->server->settings.family, (*this->ip).c_str(), &this->server->hint.sin_addr);

            bind(this->server->socket, (sockaddr*) &this->server->hint, sizeof(this->server->hint));

            //set status / error code
            this->last_error_code = ERROR_CODE::NO_ERROR;
            this->status          = STATUS_CODE::RUNNING;

            //listen
            Logger::Log("Listening for connections.",
                        __FILE__, __LINE__);
            listen(this->server->socket, this->server->settings.max_conn);

            //accept connection
            while(1)
            {
                eb_client* client = new eb_client();
                (*client).socket  = accept(this->server->socket, (sockaddr*) &(*client).hint, &(*client).client_size);

                Logger::Log("A client has connected.",
                            __FILE__, __LINE__);

                std::thread client_thread(&Server::client_handler, this, client);
                client_thread.detach();
            }
        }

        void Server::stop()
        {
            if(this->status & STATUS_CODE::RUNNING)
            {
                Logger::Log("Stopping the server.",
                            __FILE__, __LINE__);
                close(this->server->socket);

                this->status = STATUS_CODE::IDLE;
            }
        }

        void Server::send_data(eb_client   const * const client,
                               std::string const * const data)
        {
            Logger::Log("Sending data to client.",
                        __FILE__, __LINE__);
            send((*client).socket, (*data).c_str(), (*data).length(), 0x00);
        }

        //--------------------------------- Function Definations
        //---------------- Private
        void Server::client_handler(eb_client *const client)
        {
            char* buffer = new char[this->server->settings.buffer_size];

            this->protocol->add_param("server", this);
            this->protocol->add_param("client", client);

            while(this->status & STATUS_CODE::RUNNING)
            {
                memset(buffer, 0, this->server->settings.buffer_size);

                int recv_amount = recv((*client).socket, buffer, this->server->settings.buffer_size, 0x00);

                if(recv_amount == -1 || recv_amount == 0)
                {
                    Logger::Log("A client has disconnected.",
                                __FILE__, __LINE__);
                    break;
                }

                std::string client_data = std::string(buffer, 0, recv_amount);
                Logger::Log("Data recieved from client. ("+client_data+")",
                            __FILE__, __LINE__);

                this->protocol->handle(&client_data);
            }

            delete[] buffer;
            delete   client;
        }
    }
}
