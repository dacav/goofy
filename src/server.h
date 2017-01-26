#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

#include "client.h"

namespace spg
{
    class Server
    {
        public:
            Server(struct sockaddr_in & bind_addr, unsigned backlog);
            Server(struct sockaddr_in6 & bind_addr, unsigned backlog);
            ~Server();

            static Server parse_addr(const std::string &bind_addr,
                                     uint16_t port,
                                     unsigned backlog);

            const int domain;   // AF_INET, AF_INET6
            const int fd;

            Client accept();
    };

} // namespace spg
