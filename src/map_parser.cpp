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

        assert(on_nodeinfo); // otherwise, what are you parsing for?

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
            tokens.pop_front();
        }
        if (local) hostname = settings.host_name;

        uint16_t port = local ? settings.listen_port : 70;
        if (!tokens.empty()) {
            assert(tokens.size() == 1);
            try {
                port = util::strto<uint16_t>(tokens.front());
            }
            catch (Error& e) {
                throw ConfigError(std::string("Invalid port: ") + e.what());
            }
        }

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

    Loader::Loader(
            const settings::Settings& sets,
            gopher::Map& gm,
            const std::string& filename) :
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

        gopher_map.paths_map.define(filename, ""); // root entry
        gopher_map.lookup_map.mknode<gopher::NodeGopherMap>(
            gopher_map.paths_map,
            filename
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
                    add_gopherfile(std::move(info));
                    break;
                case S_IFDIR:
                    add_filesystem(std::move(info));
                    break;
                default:
                    throw ConfigError(
                        info.selector
                        + ": Unsupported file type. stat.st_mode : "
                        + std::to_string(mode)
                    );
            }
        }
        catch (IOError& e) {
            if (e.errno_was != ENOENT) throw;
            // TODO: missing local resource. Log, don't crash.
        }
    }

    void Loader::add_filesystem(gopher::NodeInfo&& info)
    {
        if (gopher_map.paths_map.define(info.selector)) {
            //gopher_map.mknode<gopher::NodeFSys>(
            //    
            //);
        }
    }

    void Loader::add_gopherfile(gopher::NodeInfo&& info)
    {
        if (gopher_map.paths_map.define(info.selector)) {
            gopher_map.lookup_map.mknode<gopher::NodeGopherMap>(
                gopher_map.paths_map,
                info.selector
            );
        }
    }

    void Loader::scan()
    {
        while (!file_reader.eof()) {
            const auto line = file_reader.next();
            parser.parse_line(line);
        }
    }

}
