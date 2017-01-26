#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

namespace spg
{
    class Client
    {
        public:
            Client(struct sockaddr_in & bind_addr, int fd);
            Client(struct sockaddr_in6 & bind_addr, int fd);
            ~Client();

            const int domain;   // AF_INET, AF_INET6
            const int fd;
    };

} // namespace spg
