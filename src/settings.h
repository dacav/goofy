#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <map>

#include <unistd.h>
#include <arpa/inet.h>

namespace goofy::settings
{
    struct ConfItemBase
    {
        ConfItemBase(const char* n) : name(n) {}

        const char* name;
        virtual void store_to(std::FILE*) const = 0;
        virtual void parse_assign(const char* line, size_t len) = 0;
    };

    template <typename Type>
    struct ConfItem : public ConfItemBase
    {
        Type value;

        ConfItem(
                std::map<std::string, ConfItemBase*>& confmap,
                const char* n,
                Type v) :
            ConfItemBase(n),
            value(v)
        {
            confmap[n] = this;
        }

        const Type& read() const
        {
            return value;
        }

        void store_to(std::FILE* f) const override;
        void parse_assign(const char* line, size_t len) override;
    };

    struct sockaddr_storage mkaddr(const char* address, uint16_t port);
    struct sockaddr_storage mkaddr(const std::string& address, uint16_t port);

    struct Settings
    {
        std::map<std::string, ConfItemBase*> confmap;

        ConfItem<uint16_t> tcp_port;
        ConfItem<sockaddr_storage> bind_addr;
        ConfItem<std::string> host_name;
        ConfItem<unsigned> listen_backlog;
        ConfItem<bool> sock_reusable;

        // Uses compile-time defaults
        Settings();
        // Uses settings from file
        Settings(const std::string& path);

        void save(const std::string& path);

    };


}
