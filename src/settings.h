#pragma once

#include <cstdint>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>

namespace spg::settings
{
    struct Settings
    {
        sockaddr_storage bind_addr;
        uint16_t listen_port;
        const std::string host_name;
        unsigned accept_backlog;
        bool sock_reusable;
    };

    struct sockaddr_storage mkaddr(const char* address, uint16_t port);
    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port);

}
