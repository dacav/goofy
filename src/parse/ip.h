#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

namespace spg::parse
{

    struct sockaddr* ipaddr (
        struct sockaddr_storage &storage,
        const char* address,
        uint16_t port = 0
    );

    struct sockaddr* ipaddr (
        struct sockaddr_storage &storage,
        const std::string& address,
        uint16_t port = 0
    );

} // namespace spg::parse
