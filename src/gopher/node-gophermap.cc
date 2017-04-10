#include "node-gophermap.h"
#include "../util/fileread.h"
#include "../map_parser.h"

#include <array>
using namespace std::placeholders;

namespace goofy::gopher
{

    NodeGopherMap::NodeGopherMap(
            const settings::Settings& sets,
            const VirtualPathsMap& vps,
            const std::string& path) :
        Node(
            NodeType::NT_MENU,
            "display", // Good for a flat map of the gophersite?
            vps.virtual_path_of(path),
            sets.self.hostname,
            sets.self.port
        ),
        settings(sets),
        vpaths(vps),
        file_path(std::move(path))
    {
        // assert isfile(file_path)
    }

    std::unique_ptr<proto::Writer> NodeGopherMap::make_writer(
            const proto::WriteParams& wp,
            const request::Request& request) const
    {
        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        goofy::util::Reader file_reader;

        const map_parser::Parser map_parser(
            settings,
            [this, writer](const map_parser::Parser::LocalNode &node) {
                writer->node(NodeInfo(
                    NodeType(node.type),
                    (std::string) node.display_name,
                    vpaths.virtual_path_of(node.selector),
                    settings.self.hostname,
                    settings.self.port
                ));
            },
            [writer](const map_parser::Parser::RemoteNode& node) {
                writer->node(NodeInfo(
                    NodeType(node.type),
                    (std::string) node.display_name,
                    (std::string) node.selector,
                    (std::string) node.hostname,
                    node.port
                ));
            },
            [this, writer](const map_parser::Parser::Url& url) {
                writer->node(NodeInfo(
                    NodeType::NT_HYPERTEXT,
                    (std::string) url.display_name,
                    (std::string) url.href,
                    settings.self.hostname,
                    settings.self.port
                ));
            },
            [writer](const util::StrRef& text) {
                writer->text((std::string)text);
            }
        );

        file_reader.feed(file_path);
        while (!file_reader.eof()) {
            map_parser.parse_line(file_reader.next());
        }

        return out;
    }

}
