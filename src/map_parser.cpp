#include "map_parser.h"
#include "error.h"

#include "gopher/node-types.h"
#include "gopher/node-fsys.h"
#include "gopher/node-gophermap.h"

#include <deque>

namespace spg::map_parser
{
    Parser::Parser(
            const settings::Settings& sets,
            const GotLocalNodeCallback oln,
            const GotRemoteNodeCallback orn,
            const GotUrlCallback ou,
            const GotTextCallback ot) :
        settings(sets),
        on_local_node(oln),
        on_remote_node(orn),
        on_url(ou),
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

        auto display_name = tokens.front();
        if (!display_name || display_name.len < 2) {
            // invalid display_name: too short. Must be at least two char.
            // TODO: Probably we want to log this.
            if (on_text) on_text(line);
            return;
        }

        // If false, what are you parsing for?
        assert(on_local_node || on_remote_node);

        char type = display_name.start[0];
        display_name ++;
        tokens.pop_front();

        auto selector = tokens.front();
        tokens.pop_front();

        if (std::string(selector).find("URL:") == 0) {
            if (on_url) {
                Url url = {
                    .display_name = display_name,
                    .href = selector
                };
                on_url(url);
            }
            return;
        }

        bool local = true;
        util::StrRef hostname;
        if (tokens.size() > 0) {
            if (tokens.front()) {
                hostname = tokens.front();
                local = false;
            }
            tokens.pop_front();
        }

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

        if (local && on_local_node) {
            LocalNode info = {
                .type = type,
                .display_name = display_name,
                .selector = selector,
            };
            on_local_node(info);
        }
        if (!local && on_remote_node) {
            RemoteNode info = {
                .type = type,
                .display_name = display_name,
                .selector = selector,
                .hostname = hostname,
                .port = port,
            };
            on_remote_node(info);
        }
    }

    Loader::Loader(
            const settings::Settings& sets,
            gopher::Map& gm,
            const gopher::GopherTypeGuesser& tg,
            const std::string& filename) :
        settings(sets),
        gopher_map(gm),
        type_guesser(tg),
        parser(
            settings,
            std::bind(
                &Loader::got_local_node,
                this,
                std::placeholders::_1
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

    void Loader::got_local_node(const Parser::LocalNode& info)
    {
        try {
            mode_t mode = util::mode_of(info.selector);

            switch (mode & S_IFMT) {
                case S_IFREG:
                    // TODO: here just assuming the file is a gophermap.
                    // Probably wanna check more?
                    add_gopherfile((info));
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

    void Loader::add_filesystem(const Parser::LocalNode& localnode)
    {
        if (gopher_map.paths_map.define(localnode.selector)) {
            //gopher_map.mknode<gopher::NodeFSys>(
            //    
            //);
        }
    }

    void Loader::add_gopherfile(const Parser::LocalNode& localnode)
    {
        if (gopher_map.paths_map.define(localnode.selector)) {
            gopher_map.lookup_map.mknode<gopher::NodeGopherMap>(
                gopher_map.paths_map,
                localnode.selector
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
