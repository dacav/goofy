#pragma once

#include <string>
#include <cstdio>
#include <memory>
#include <unordered_map>

#include "settings.h"

#include "gopher/map.h"
#include "gopher/node-menu.h"
#include "gopher/node-types.h"

#include "util/fileread.h"
#include "util/str.h"

namespace spg::map_parser
{
    class Parser
    {
        public:
            struct LocalNode
            {
                char type;
                spg::util::StrRef display_name;
                spg::util::StrRef selector;
            };
            using GotLocalNodeCallback = std::function<void(const LocalNode&)>;

            struct RemoteNode
            {
                char type;
                spg::util::StrRef display_name;
                spg::util::StrRef selector;
                spg::util::StrRef hostname;
                uint16_t port;
            };
            using GotRemoteNodeCallback = std::function<void(const RemoteNode&)>;

            struct Url
            {
                spg::util::StrRef display_name;
                spg::util::StrRef href;
            };
            using GotUrlCallback = std::function<void(const Url&)>;

            using GotTextCallback = std::function<void(const spg::util::StrRef&)>;

            Parser(
                const settings::Settings& settings,
                const GotLocalNodeCallback on_local_node=nullptr,
                const GotRemoteNodeCallback on_remote_node=nullptr,
                const GotUrlCallback on_url=nullptr,
                const GotTextCallback on_text=nullptr
            );

            void parse_line(const util::StrRef& ref) const;

        private:
            const settings::Settings& settings;
            const GotLocalNodeCallback on_local_node;
            const GotRemoteNodeCallback on_remote_node;
            const GotUrlCallback on_url;
            const GotTextCallback on_text;
    };

    class Loader
    {
        public:
            Loader(
                const settings::Settings& settings,
                gopher::Map& gopher_map,
                const gopher::GopherTypeGuesser& type_guesser,
                const std::string& filename
            );

        private:
            const settings::Settings& settings;
            gopher::Map& gopher_map;
            const gopher::GopherTypeGuesser& type_guesser;
            Parser parser;
            util::Reader file_reader;

            void got_local_node(const Parser::LocalNode&);
            void add_gopherfile(const Parser::LocalNode&);
            void add_filesystem(const Parser::LocalNode&);
            void scan();
    };

}
