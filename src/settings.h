#pragma once

#include <cstdint>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>

namespace goofy::settings
{
    struct Settings
    {
        sockaddr_storage bind_addr;
        uint16_t listen_port;
        std::string host_name;
        unsigned listen_backlog;
        bool sock_reusable;
    };

    struct sockaddr_storage mkaddr(const char* address, uint16_t port);
    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port);

}
