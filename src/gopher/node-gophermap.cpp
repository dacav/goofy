#include "node-gophermap.h"
#include "../util/fileread.h"
#include "../map_parser.h"

#include <array>

namespace spg::gopher
{

    NodeGopherMap::NodeGopherMap(
            const settings::Settings& sets,
            std::string&& path,
            NodeInfo&& info) :
        Node(
            NodeType::NT_MENU,
            std::move(info.display_name),
            std::move(info.selector),
            std::move(info.host),
            info.port
        ),
        settings(sets),
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
        // if local, yada
        writer->node(node);
    }

}
