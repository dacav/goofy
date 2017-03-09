#include "map_parser.h"
#include "error.h"

#include "gopher/node-types.h"

#include <deque>

#include <iostream> 
namespace spg::map_parser
{
    Parser::Parser(
            const spg::settings::Settings& sets,
            const GotNodeInfoCallback on,
            const GotEOFCallback oe,
            const GotTextCallback ot) :
        settings(sets),
        on_node(on),
        on_eof(oe),
        on_text(ot)
    {
    }

    void Parser::parse_line(const spg::util::StrRef& line)
    {
        using namespace spg::util;

        auto tokens = tokenize(line, '\t');

        if (tokens.size() == 1) {
            if (on_text) on_text(line);
            return;
        }
        if (tokens.size() > 4) {
            if (on_text) on_text(line);
        }

        char type;
        std::string display_name;
        {
            spg::util::StrRef& front = tokens.front();
            if (!front || front.len < 2) {
                if (on_text) on_text(line);
            }

            type = front.start[0];
            display_name = std::string(front.start + 1, front.len - 1);
            tokens.pop_front();
        }

        std::string selector = tokens.front();
        tokens.pop_front();

        std::string hostname;
        if (tokens.size() > 0) {
            if (tokens.front()) {
                hostname = (std::string) tokens.front();
            }
            else {
                hostname = settings.host_name;
            }
            tokens.pop_front();
        }

        uint16_t port = 70;
        if (!tokens.empty()) {
            assert(tokens.size() == 1);
            try {
                port = spg::util::strto<uint16_t>(tokens.front());
            }
            catch (Error& e) {
                throw ConfigError(std::string("Invalid port: ") + e.what());
            }
        }

        if (on_node) {
            on_node(gopher::NodeInfo(
                gopher::NodeType(type),
                std::move(display_name),
                std::move(selector),
                std::move(hostname),
                port
            ));
        }
    }

    Loader::Loader(
            const spg::settings::Settings& settings,
            gopher::Map& gopher_map,
            const std::string& filename) :
        Loader(settings, gopher_map, filename.c_str())
    {}

    Loader::Loader(
            const spg::settings::Settings& sets,
            gopher::Map& gm,
            const char* filename) :
        settings(sets),
        gopher_map(gm),
        parser(
            settings,
            std::bind(&Loader::got_node, this, std::placeholders::_1)
        )
    {
        file_reader.feed(filename);
        scan();

        // Goal: retrieve gopher links inside the file, and add them to the
        // map.
        // We seek for
        // - paths of other gopherfiles
        //   * they must be added to the map with a node-gophermap type
        // - paths of directories
        //   * they must be added to the map as node-fsys
        //   * they must be scanned entirely for other gopherfiles
        // All other lines can be ignored: this is not node-gophermap...
    }

    void Loader::got_node(const gopher::NodeInfo& info)
    {
        std::cerr << "Got node: " << info.type << " sel=" << info.selector
            << " disp=" << info.display_name << std::endl;
    }

    void Loader::scan()
    {
        while (!file_reader.eof()) {
            const auto line = file_reader.next();
            parser.parse_line(line);
        }
    }

}
