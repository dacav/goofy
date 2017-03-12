#include "map_parser.h"
#include "error.h"

#include "gopher/node-types.h"
#include "gopher/node-fsys.h"
#include "gopher/node-gophermap.h"

#include <deque>

#include <iostream> 
namespace spg::map_parser
{
    Parser::Parser(
            const settings::Settings& sets,
            const GotNodeInfoCallback on,
            const GotTextCallback ot) :
        settings(sets),
        on_nodeinfo(on),
        on_text(ot)
    {
    }

    void Parser::parse_line(const util::StrRef& line) const
    {
        auto tokens = tokenize(line, '\t');

        if (tokens.size() == 1) {
            if (on_text) on_text(line);
            return;
        }
        if (tokens.size() > 4) {
            if (on_text) on_text(line);
            return;
        }

        char type;
        std::string display_name;
        {
            util::StrRef& front = tokens.front();
            if (!front || front.len < 2) {
                if (on_text) on_text(line);
            }

            type = front.start[0];
            display_name = std::string(front.start + 1, front.len - 1);
            tokens.pop_front();
        }

        std::string selector = tokens.front();
        tokens.pop_front();

        bool local = true;
        std::string hostname;
        if (tokens.size() > 0) {
            if (tokens.front()) {
                hostname = (std::string) tokens.front();
                local = false;
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
                port = util::strto<uint16_t>(tokens.front());
            }
            catch (Error& e) {
                throw ConfigError(std::string("Invalid port: ") + e.what());
            }
        }

        if (on_nodeinfo) {
            on_nodeinfo(
                gopher::NodeInfo(
                    gopher::NodeType(type),
                    std::move(display_name),
                    std::move(selector),
                    std::move(hostname),
                    port
                ),
                local
            );
        }
    }

    Loader::Loader(
            const settings::Settings& settings,
            gopher::Map& gopher_map,
            const std::string& filename) :
        Loader(settings, gopher_map, filename.c_str())
    {}

    Loader::Loader(
            const settings::Settings& sets,
            gopher::Map& gm,
            const char* filename) :
        settings(sets),
        gopher_map(gm),
        parser(
            settings,
            std::bind(
                &Loader::got_nodeinfo,
                this,
                std::placeholders::_1,
                std::placeholders::_2
            )
        )
    {
        file_reader.feed(filename);

        gopher_map.mknode<gopher::NodeGopherMap>(
            filename,
            gopher::NodeInfo(
                gopher::NodeType::NT_MENU,
                "root",
                "",     // The first gophermap file is the server root.
                settings.host_name,
                settings.listen_port
            )
        );
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

    void Loader::got_nodeinfo(gopher::NodeInfo&& info, bool local)
    {
        if (!local) return; // remote servers don't need to be looked up

        try {
            mode_t mode = gopher::mode_of(info.selector);

            switch (mode & S_IFMT) {
                case S_IFREG:
                    // TODO: here just assuming the file is a gophermap.
                    // Probably wanna check more?
                    gopher_map.mknode<gopher::NodeGopherMap>(
                        virtual_selector_for(info.selector),
                        std::move(info)
                    );
                    break;
                case S_IFDIR:
                    // TODO: mknode of a node-fsys
                    break;
                default:
                    throw ConfigError(
                        info.selector + ": Unsupported file type"
                    );
            }
        }
        catch (IOError& e) {
            if (e.errno_was != ENOENT) throw;
            // TODO: missing local resource. Log, don't crash.
        }
    }

    void Loader::scan()
    {
        while (!file_reader.eof()) {
            const auto line = file_reader.next();
            parser.parse_line(line);
        }
    }

    std::string Loader::virtual_selector_for(const std::string& path)
    {
        auto seek = path_to_selector.find(path);
        if (seek == path_to_selector.end()) {
            std::string out = std::to_string(path_to_selector.size());
            path_to_selector[path] = out;
            return out;
        }
        else {
            return seek->second;
        }
    }

}
