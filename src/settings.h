#pragma once

#include <cstdint>
#include <string>

#include <unistd.h>
#include <arpa/inet.h>

namespace goofy::settings
{
    template <typename Type>
    struct ConfItem
    {
        Type value;
        const char* name;
        static const size_t size = sizeof(Type);

        ConfItem(const char* n, Type v) :
            name(n),
            value(v)
        {
        }

        operator const Type& () const
        {
            return value;
        }

    };

    struct sockaddr_storage mkaddr(const char* address, uint16_t port);
    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port);

    struct Settings
    {
        ConfItem<uint16_t> tcp_port;
        ConfItem<sockaddr_storage> bind_addr;
        ConfItem<std::string> host_name;
        ConfItem<unsigned> listen_backlog;
        ConfItem<bool> sock_reusable;

        Settings() :
            tcp_port("net.tcp_port", 7070),
            bind_addr("net.bind_addr", mkaddr("::1", tcp_port)),
            host_name("net.host_name", "localhost"),
            listen_backlog("net.listen_backlog", 10),
            sock_reusable("net.sock_reusable", true)
        {
        }
    };


}
