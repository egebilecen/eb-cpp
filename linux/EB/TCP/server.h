#pragma once

#include <string>
#include <thread>
#include <utility>
#include <vector>

// includes for socket
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "../Protocol/protocol.h"
#include "../Logger/logger.h"

namespace EB
{
    namespace TCP
    {
        class Server
        {
            public:
                //--------------------------------- Type Defines
                typedef std::pair<std::string, std::string> KeyValPair;

                //--------------------------------- Structures
                //---------------- Socket
                struct eb_client
                {
                    sockaddr_in hint;
                    socklen_t   client_size = sizeof(hint);
                    int         socket;
                };
                //--------------------------------- Enumerations
                enum ERROR_CODE
                {
                    NO_ERROR      = 0,
                    SOCKET_CREATE,
                    RECV,
                    SERVER_RUNNING
                };

                enum STATUS_CODE
                {
                    IDLE    = 0x00,
                    RUNNING = 0x01
                };

                //--------------------------------- Constructor
                Server(std::string         const* ip,
                       uint16_t            const* port,
                       EB::Protocol::Base *const  protocol,
                       bool                       debug = false);

                //--------------------------------- Public Function(s)
                //---------------- Server
                void start();
                void stop();
                void send_data(eb_client   const * const client,
                               std::string const * const data);

            private:
                //--------------------------------- Structures
                struct socket_settings
                {
                    //default settings
                    int family      = AF_INET;
                    int type        = SOCK_STREAM;
                    int max_conn    = SOMAXCONN;
                    int buffer_size = 1025;
                };

                //---------------- Socket
                struct eb_server
                {
                    int         socket;
                    sockaddr_in hint;

                    socket_settings settings;
                };

                //--------------------------------- Variables
                //---------------- Server
                std::string          const* ip;
                uint16_t             const* port;
                EB::Protocol::Base  *const  protocol;
                eb_server           *const  server = new eb_server();

                //---------------- Status&Error Codes etc.
                bool debug;
                int  last_error_code = ERROR_CODE::NO_ERROR;
                int  status          = STATUS_CODE::IDLE;

                //---------------- Private Function(s)
                void client_handler(eb_client *const client);
        };
    }
}
