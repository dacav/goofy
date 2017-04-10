#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <unistd.h>
#include <arpa/inet.h>

#include <libconfig.h++>

namespace goofy::settings
{
    struct Settings
    {

        /* Settings about network configuration */
        struct Network
        {
            sockaddr_storage bind_address;
            unsigned listen_backlog;
            bool sock_reusable;

            Network();
            void save_to(libconfig::Setting&) const;
            void load_from(const libconfig::Setting&);
        };


        /* How the server introduces itself over gopher */
        struct Self
        {
            std::string hostname;
            uint16_t port;

            Self();
            void save_to(libconfig::Setting&) const;
            void load_from(const libconfig::Setting&);
        };

        struct Content
        {
            std::string root_gophermap;

            Content();
            void save_to(libconfig::Setting&) const;
            void load_from(const libconfig::Setting&);
        };

        Network network;
        Content content;
        Self self;

        // Uses compile-time defaults
        Settings() = default;
        // Uses settings from file
        Settings(const char* path);
        Settings(const std::string& path);

        void save(const char* path);
        void save(const std::string& path);
    };


}
