#include <unistd.h>
#include <arpa/inet.h>
#include <cassert>

#include "error.h"

namespace
{

    struct sockaddr_in * parse4(
            struct sockaddr_storage &storage,
            const char* addr,
            uint16_t port)
    {
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

    struct sockaddr_in6 * parse6(
            struct sockaddr_storage &storage,
            const char* addr,
            uint16_t port)
    {
        //Implicitly zeroed: in6.sin6_flowinfo, in6.sin6_scope_id
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
                return &in6;
            default:
                assert(0);
        }
    }

} // anon namespace

namespace spg::settings
{

    struct sockaddr_storage mkaddr(const char* address, uint16_t port)
    {
        struct sockaddr_storage storage;
        if (parse4(storage, address, port) == nullptr
                && parse6(storage, address, port) == nullptr) {
            throw spg::Error(std::string("Cannot parse address: ") + address);
        }
        return storage;
    }

    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port)
    {
        return mkaddr(address.c_str(), port);
    }

}
