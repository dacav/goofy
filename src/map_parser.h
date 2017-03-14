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
            using GotNodeInfoCallback = std::function<void(gopher::NodeInfo&&, bool)>;

            Parser(
                const settings::Settings& settings,
                const GotNodeInfoCallback on_nodeinfo=nullptr,
                const GotTextCallback on_text=nullptr
            );

            void parse_line(const util::StrRef& ref) const;

        private:
            const settings::Settings& settings;
            const GotNodeInfoCallback on_nodeinfo;
            const GotTextCallback on_text;
    };

    class Loader
    {
        public:
            Loader(
                const settings::Settings& settings,
                gopher::Map& gopher_map,
                const std::string& filename
            );

        private:
            const settings::Settings& settings;
            gopher::Map& gopher_map;
            Parser parser;
            util::Reader file_reader;

            void got_nodeinfo(gopher::NodeInfo&&, bool local);
            void add_gopherfile(gopher::NodeInfo&& info);
            void add_filesystem(gopher::NodeInfo&& info);
            void scan();
    };

}
