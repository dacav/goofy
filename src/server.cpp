#include "server.h"
#include "error.h"

#include <unistd.h>
#include <arpa/inet.h>

#include <cassert>
#include <cstring>

namespace
{

    template <typename SockAddr, int AddrFamily>
    int serve(const SockAddr & bind_addr, unsigned backlog)
    {
        int fd = socket(AddrFamily, SOCK_STREAM, 0);
        if (fd == -1) {
            throw spg::Error("socket", errno);
        }

        socklen_t socklen = sizeof(SockAddr);
        if (bind(fd, reinterpret_cast<const struct sockaddr *>(&bind_addr),
                 socklen) == -1) {
            int e = errno;
            close(fd);
            throw spg::Error("bind", e);
        }

        if (listen(fd, backlog) == -1) {
            int e = errno;
            close(fd);
            throw spg::Error("listen", e);
        }

        return fd;
    }

    struct sockaddr_in * parse4(struct sockaddr_storage &storage,
                                const std::string & bind_addr,
                                uint16_t port)
    {
        const char *addr = bind_addr.c_str();

        std::memset(&storage, 0, sizeof storage);
        struct sockaddr_in& in4 = reinterpret_cast<struct sockaddr_in &>(
            storage
        );
        switch (inet_pton(AF_INET, addr, &in4.sin_addr)) {
            case -1:
                throw spg::Error("inet_pton(AF_INET, ...)", errno);
            case 0:
                return nullptr;
            case 1:
                in4.sin_family = AF_INET;
                in4.sin_port = htons(port);
                return &in4;
            default:
                assert(0);
        }
    }

    struct sockaddr_in6 * parse6(struct sockaddr_storage &storage,
                                 const std::string & bind_addr,
                                 uint16_t port)
    {
        const char *addr = bind_addr.c_str();

        std::memset(&storage, 0, sizeof storage);
        struct sockaddr_in6& in6 = reinterpret_cast<struct sockaddr_in6 &>(
            storage
        );
        switch (inet_pton(AF_INET6, addr, &in6.sin6_addr)) {
            case -1:
                throw spg::Error("inet_pton(AF_INET6, ...)", errno);
            case 0:
                return nullptr;
            case 1:
                in6.sin6_family = AF_INET6;
                in6.sin6_port = htons(port);
                //in6.sin6_flowinfo = ?;    // TODO: use of this?
                //in6.sin6_scope_id = ?;    // TODO: use of this?
                return &in6;
            default:
                assert(0);
        }
    }

} // anonymous namespace

namespace spg
{

    Server::Server(struct sockaddr_in & bind_addr, unsigned backlog)
        : domain(AF_INET)
        , fd(serve<struct sockaddr_in, AF_INET>(bind_addr, backlog))
    {
    }

    Server::Server(struct sockaddr_in6 & bind_addr, unsigned backlog)
        : domain(AF_INET6)
        , fd(serve<struct sockaddr_in6, AF_INET6>(bind_addr, backlog))
    {
    }

    Server Server::parse_addr(const std::string &bind_addr,
                              uint16_t port,
                              unsigned backlog)
    {
        struct sockaddr_storage storage;

        struct sockaddr_in *in4 = parse4(storage, bind_addr, port);
        if (in4 != nullptr) {
            return Server(*in4, backlog);
        }

        struct sockaddr_in6 *in6 = parse6(storage, bind_addr, port);
        if (in6 != nullptr) {
            return Server(*in6, backlog);
        }

        throw Error(std::string("Unrecognized address: ") + bind_addr);
    }

    Client Server::accept()
    {
        struct sockaddr_storage storage;
        socklen_t socklen = sizeof storage;

        int clfd = ::accept(
            fd,
            reinterpret_cast<struct sockaddr *>(&storage),
            &socklen
        );

        if (clfd == -1) {
            throw Error("accept", errno);
        }

        if (domain == AF_INET) {
            return Client(
                reinterpret_cast<struct sockaddr_in &>(storage),
                clfd
            );
        }
        if (domain == AF_INET6) {
            return Client(
                reinterpret_cast<struct sockaddr_in6 &>(storage),
                clfd
            );
        }

        assert(false);
    }

    Server::~Server()
    {
        close(fd);
    }

} // namespace spg
