#include <unistd.h>
#include <arpa/inet.h>
#include <cassert>
#include <memory>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>

#include <libconfig.h++>

#include "error.h"
#include "settings.h"

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
                throw goofy::Error("inet_pton(AF_INET, ...)", errno);
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
                throw goofy::Error("inet_pton(AF_INET6, ...)", errno);
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

    struct sockaddr_storage mkaddr(const char* address, uint16_t port)
    {
        struct sockaddr_storage storage;
        if (parse4(storage, address, port) == nullptr
                && parse6(storage, address, port) == nullptr) {
            throw goofy::ConfigError(std::string("Cannot parse address: ") + address);
        }
        return storage;
    }

    using LCType = libconfig::Setting::Type;

} // anon namespace

namespace goofy::settings
{
    Settings::Network::Network() :
        bind_address(mkaddr("::", 70)),
        listen_backlog(10)
    {
    }

    void Settings::Network::save_to(libconfig::Setting& group) const
    {
        const struct sockaddr& addr = reinterpret_cast<const struct sockaddr&>(bind_address);
        const int af = addr.sa_family;

        in_port_t port;
        size_t buflen;
        const void *parse_addr;
        switch (af) {
            case AF_INET: {
                const sockaddr_in& addr_in = reinterpret_cast<const sockaddr_in&>(addr);
                buflen = INET_ADDRSTRLEN;
                port = addr_in.sin_port;
                parse_addr = &addr_in.sin_addr;
                break;
            }
            case AF_INET6: {
                const sockaddr_in6& addr_in6 = reinterpret_cast<const sockaddr_in6&>(addr);
                buflen = INET6_ADDRSTRLEN;
                port = addr_in6.sin6_port;
                parse_addr = &addr_in6.sin6_addr;
                break;
            }
            default:
                assert(0); // did we invent more?
        }

        std::unique_ptr<char[]> buffer(new char[buflen]);
        const char* out = inet_ntop(af, parse_addr, buffer.get(), buflen);
        if (out == nullptr) {
            throw ConfigError("inet_ntop", errno);
        }

        group.add("address", libconfig::Setting::Type::TypeString) = out;
        group.add("port", libconfig::Setting::Type::TypeInt) = ntohs(port);
        group.add("listen_backlog", libconfig::Setting::Type::TypeInt) = int(listen_backlog);
        group.add("sock_reusable", libconfig::Setting::Type::TypeBoolean) = bool(sock_reusable);
    }

    void Settings::Network::load_from(const libconfig::Setting& group)
    {
        bind_address = mkaddr(
            group.lookup("address"),
            unsigned(group.lookup("port"))
        );
        listen_backlog = unsigned(group.lookup("listen_backlog"));
        sock_reusable = bool(group.lookup("sock_reusable"));
    }

    Settings::Self::Self() :
        hostname("localhost"),
        port(70)
    {
    }

    void Settings::Self::load_from(const libconfig::Setting& group)
    {
        hostname = (const char*) group.lookup("hostname");
        port = unsigned(group.lookup("port"));
    }

    void Settings::Self::save_to(libconfig::Setting& group) const
    {
        group.add("hostname", LCType::TypeString) = hostname;
        group.add("port", LCType::TypeInt) = port;
    }

    Settings::Content::Content() :
        root_gophermap("path/to/gophermap")
    {
    }

    void Settings::Content::save_to(libconfig::Setting& group) const
    {
        group.add("root_gophermap", LCType::TypeString) = root_gophermap;
    }

    void Settings::Content::load_from(const libconfig::Setting& group)
    {
        root_gophermap = (const char*)group.lookup("root_gophermap");
    }

    Settings::Settings(const std::string& path) :
        Settings(path.c_str())
    {
    }

    Settings::Settings(const char* path)
    {
        libconfig::Config cfg;
        cfg.readFile(path);
        auto& root = cfg.getRoot();
        network.load_from(root.lookup("network"));
        self.load_from(root.lookup("server"));
        content.load_from(root.lookup("content"));
    }

    void Settings::save(const char* path)
    {
        libconfig::Config cfg;
        auto& root = cfg.getRoot();
        network.save_to(root.add("network", LCType::TypeGroup));
        self.save_to(root.add("server", LCType::TypeGroup));
        content.save_to(root.add("content", LCType::TypeGroup));
        cfg.writeFile(path);
    }

    void Settings::save(const std::string& path)
    {
        save(path.c_str());
    }

}
