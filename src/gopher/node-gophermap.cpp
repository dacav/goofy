#include "node-gophermap.h"
#include "../util/fileread.h"
#include "../map_parser.h"

#include <array>

namespace spg::gopher
{

    NodeGopherMap::NodeGopherMap(
            const settings::Settings& sets,
            const VirtualPathsMap& vps,
            const std::string& path) :
        Node(
            NodeType::NT_MENU,
            "display", // Good for a flat map of the gophersite?
            vps.virtual_path_of(path),
            sets.host_name,
            sets.listen_port
        ),
        settings(sets),
        vpaths(vps),
        file_path(std::move(path)),
        map_parser(
            settings,
            std::bind(&NodeGopherMap::got_nodeinfo, this,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            std::bind(&NodeGopherMap::got_text, this,
                std::placeholders::_1
            )
        )
    {
        // assert isfile(file_path)
    }

    std::unique_ptr<proto::Writer> NodeGopherMap::make_writer(
            const proto::WriteParams& wp,
            const request::Request& request)
    {
        writer.reset(new proto::MenuWriter(wp));

        spg::util::Reader file_reader;

        file_reader.feed(file_path);
        while (!file_reader.eof()) {
            map_parser.parse_line(file_reader.next());
        }

        return std::unique_ptr<proto::Writer>(writer.release());
    }

    void NodeGopherMap::got_text(std::string&& msg)
    {
        writer->text(msg);
    }

    void NodeGopherMap::got_nodeinfo(gopher::NodeInfo&& node, bool local)
    {
        if (local) {
            writer->node(NodeInfo(
                NodeType(node.type), // TODO: fix this type
                node.display_name,
                vpaths.virtual_path_of(node.selector),
                node.host,
                node.port
            ));
        }
        else {
            writer->node(node);
        }
    }

}
