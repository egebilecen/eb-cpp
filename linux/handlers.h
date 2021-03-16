// Contains functions for client data handling.
#pragma once

#include "EB/TCP/server.h"
#include "EB/Protocol/protocol.h"

inline void hello(EB::Protocol::Base* const protocol)
{
    EB::TCP::Server*            server = (EB::TCP::Server*)            protocol->get_param("server");
    EB::TCP::Server::eb_client* client = (EB::TCP::Server::eb_client*) protocol->get_param("client");

    if(server == nullptr || client == nullptr) return;

    std::string data = "merhaba dunya";
    server->send_data(client, &data);
}
