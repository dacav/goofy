#include <unistd.h>
#include <arpa/inet.h>
#include <cassert>
#include <memory>

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

} // anon namespace

namespace goofy::settings
{
    Settings::Settings() :
        tcp_port(confmap, "net.tcp_port", 7070),
        bind_addr(confmap, "net.bind_addr", mkaddr("::1", tcp_port.read())),
        host_name(confmap, "net.host_name", "localhost"),
        listen_backlog(confmap, "net.listen_backlog", 10),
        sock_reusable(confmap, "net.sock_reusable", true)
    {
    }

    Settings::Settings(const std::string& path) :
        Settings()
    {
        auto file = fopen(path.c_str(), "rt");
        if (file == nullptr) {
            throw IOError("Opening" + path, errno);
        }
        std::unique_ptr<FILE, int(*)(FILE*)> raii(file, fclose);
    }

    void Settings::save(const std::string& path)
    {
        auto file = fopen(path.c_str(), "wt");
        if (file == nullptr) {
            throw IOError("Opening" + path, errno);
        }
        std::unique_ptr<FILE, int(*)(FILE*)> raii(file, fclose);

        for (auto& pair : confmap) {
            pair.second->store_to(file);
        }
    }

    struct sockaddr_storage mkaddr(const char* address, uint16_t port)
    {
        struct sockaddr_storage storage;
        if (parse4(storage, address, port) == nullptr
                && parse6(storage, address, port) == nullptr) {
            throw goofy::Error(std::string("Cannot parse address: ") + address);
        }
        return storage;
    }

    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port)
    {
        return mkaddr(address.c_str(), port);
    }

    template <>
    void ConfItem<uint16_t>::store_to(std::FILE* f) const
    {
        fprintf(f, "%s %hu\n", name, value);
    }

    template <>
    void ConfItem<sockaddr_storage>::store_to(std::FILE* f) const
    {
        fprintf(f, "%s ...not trivial\n", name);
    }

    template <>
    void ConfItem<std::string>::store_to(std::FILE* f) const
    {
        fprintf(f, "%s %s\n", name, value.c_str());
    }

    template <>
    void ConfItem<unsigned>::store_to(std::FILE* f) const
    {
        fprintf(f, "%s %u\n", name, value);
    }

    template <>
    void ConfItem<bool>::store_to(std::FILE* f) const
    {
        fprintf(f, "%s %s\n", name, value ? "yes" : "no");
    }

}
