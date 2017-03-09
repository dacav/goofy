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
            using GotTextCallback = std::function<void(std::string&&)>;
            using GotNodeInfoCallback = std::function<void(gopher::NodeInfo&&)>;
            using GotEOFCallback = std::function<void(void)>;

            Parser(
                const spg::settings::Settings& settings,
                const GotNodeInfoCallback on_node=nullptr,
                const GotEOFCallback on_eof=nullptr,
                const GotTextCallback on_text=nullptr
            );

            void parse_line(const spg::util::StrRef& ref);

        private:
            const spg::settings::Settings& settings;
            const GotNodeInfoCallback on_node;
            const GotEOFCallback on_eof;
            const GotTextCallback on_text;
    };

    class Loader
    {
        public:
            Loader(
                const spg::settings::Settings& settings,
                gopher::Map& gopher_map,
                const char* filename
            );
            Loader(
                const spg::settings::Settings& settings,
                gopher::Map& gopher_map,
                const std::string& filename
            );

        private:
            const spg::settings::Settings& settings;
            gopher::Map& gopher_map;
            spg::gopher::NodeMenu& root_menu;
            Parser parser;

            spg::util::Reader file_reader;

            void got_node(gopher::NodeInfo&&);
            void scan();

            std::string virtual_selector_for(const std::string& path);
            std::unordered_map<std::string, std::string> path_to_selector;
    };

}
