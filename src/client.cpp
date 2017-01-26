#include "client.h"
#include <unistd.h>

namespace spg
{
    Client::Client(struct sockaddr_in & bind_addr, int _fd)
        : domain(AF_INET)
        , fd(_fd)
    {
    }

    Client::Client(struct sockaddr_in6 & bind_addr, int _fd)
        : domain(AF_INET6)
        , fd(_fd)
    {
    }

    Client::~Client()
    {
        close(fd);
    }

} // namespace spg
