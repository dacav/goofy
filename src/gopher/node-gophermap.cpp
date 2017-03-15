#include "node-gophermap.h"
#include "../util/fileread.h"
#include "../map_parser.h"

#include <array>
using namespace std::placeholders;

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
            std::bind(&NodeGopherMap::got_local_node, this, _1),
            std::bind(&NodeGopherMap::got_remote_node, this, _1),
            std::bind(&NodeGopherMap::got_url, this, _1),
            std::bind(&NodeGopherMap::got_text, this, _1)
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

    void NodeGopherMap::got_text(const std::string& msg)
    {
        writer->text(msg);
    }

    void NodeGopherMap::got_url(const Url& url)
    {
        writer->node(NodeInfo(
            NodeType::NT_HYPERTEXT,
            (std::string) url.display_name,
            (std::string) url.href,
            settings.host_name,
            settings.listen_port
        ));
    }

    void NodeGopherMap::got_remote_node(const RemoteNode& node)
    {
        writer->node(NodeInfo(
            NodeType(node.type),
            (std::string) node.display_name,
            (std::string) node.selector,
            (std::string) node.hostname,
            node.port
        ));
    }

    void NodeGopherMap::got_local_node(const LocalNode& node)
    {
        writer->node(NodeInfo(
            NodeType(node.type),
            (std::string) node.display_name,
            vpaths.virtual_path_of(node.selector),
            settings.host_name,
            settings.listen_port
        ));
    }

}
