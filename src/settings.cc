#include <unistd.h>
#include <arpa/inet.h>
#include <cassert>
#include <memory>
#include <iostream>

#include "error.h"
#include "settings.h"
#include "util/fileread.h"

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
        util::Reader reader(path);
        const char* blanks = " \t";
        while (!reader.eof()) {
            util::StrRef lineref = reader.next();
            lineref.ltrim();
            if (lineref.len == 0) continue;         // skip empty line
            if (*lineref.start == '#') continue;    // skip comment

            std::string key(lineref);
            auto end_key = key.find_first_of(blanks);

            if (end_key == key.npos) {
                throw goofy::ConfigError("Invalid line: " + key);
            }

            key.erase(end_key);

            auto search = confmap.find(key);
            if (search == confmap.end()) {
                throw goofy::ConfigError("No such key: " + key);
            }

            lineref += end_key + 1;
            lineref.ltrim();
            auto& item = *(search->second);

            item.parse_assign(lineref.start, lineref.len);
        }
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
            throw goofy::ConfigError(std::string("Cannot parse address: ") + address);
        }
        return storage;
    }

    template <>
    size_t ConfItem<uint16_t>::store_to(std::FILE* f) const
    {
        const int ret = std::fprintf(f, "%s %hu\n", name, value);
        if (ret < 0) {
            throw IOError("fprintf failure");
        }
        return size_t(ret);
    }

    template <>
    void ConfItem<uint16_t>::parse_assign(const char* line, size_t len)
    {
        try {
            value = util::strto<uint16_t>(std::string(line, len));
        }
        catch(Error& e) {
            throw goofy::ConfigError("Invalid uint16: " + std::string(e.what()));
        }
    }

    template <>
    size_t ConfItem<sockaddr_storage>::store_to(std::FILE* f) const
    {
        const sockaddr& addr = reinterpret_cast<const sockaddr&>(value);
        const int af = addr.sa_family;

        in_port_t port;
        size_t buflen;
        switch (af) {
            case AF_INET:
                port = reinterpret_cast<const sockaddr_in&>(addr).sin_port;
                buflen = INET_ADDRSTRLEN;
                break;
            case AF_INET6:
                port = reinterpret_cast<const sockaddr_in6&>(addr).sin6_port;
                buflen = INET6_ADDRSTRLEN;
                break;
            default:
                assert(0); // did we invent more?
        }

        std::unique_ptr<char[]> buffer(new char[buflen]);
        const char* out = inet_ntop(af, &value, buffer.get(), buflen);
        if (out == nullptr) {
            throw ConfigError("Cannot serialize " + std::string(name), errno);
        }
        const int ret = std::fprintf(f, "%s %s %hu\n", name, out, port);
        if (ret < 0) {
            throw IOError("fprintf failure");
        }
        return size_t(ret);
    }

    template <>
    void ConfItem<sockaddr_storage>::parse_assign(const char* line, size_t len)
    {
    }

    template <>
    size_t ConfItem<std::string>::store_to(std::FILE* f) const
    {
        const int ret = std::fprintf(f, "%s %s\n", name, value.c_str());
        if (ret < 0) {
            throw IOError("fprintf failure");
        }
        return size_t(ret);
    }

    template <>
    void ConfItem<std::string>::parse_assign(const char* line, size_t len)
    {
        std::fprintf(stderr, "parse str %s\n", std::string(line, len).c_str());
    }

    template <>
    size_t ConfItem<unsigned>::store_to(std::FILE* f) const
    {
        const int ret = std::fprintf(f, "%s %u\n", name, value);
        if (ret < 0) {
            throw IOError("fprintf failure");
        }
        return size_t(ret);
    }

    template <>
    void ConfItem<unsigned>::parse_assign(const char* line, size_t len)
    {
        std::fprintf(stderr, "parse uint %s\n", std::string(line, len).c_str());
    }

    template <>
    size_t ConfItem<bool>::store_to(std::FILE* f) const
    {
        const int ret = std::fprintf(f, "%s %s\n", name, value ? "yes" : "no");
        if (ret < 0) {
            throw IOError("fprintf failure");
        }
        return size_t(ret);
    }

    template <>
    void ConfItem<bool>::parse_assign(const char* line, size_t len)
    {
        fprintf(stderr, "parse bool %s\n", std::string(line, len).c_str());
    }

}
