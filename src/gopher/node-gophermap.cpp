#include "node-gophermap.h"
#include "../util/fileread.h"

#include <array>

namespace spg::gopher
{

    NodeGopherMap::NodeGopherMap(
            const gopher::Map& m,
            std::string&& selector,
            NodeInfo&& info) :
        Node(
            NodeType::NT_MENU,
            std::move(info.display_name),
            selector,
            std::move(info.host),
            info.port
        ),
        map(m),
        file_path(std::move(info.selector))
    {
        // assert isfile(file_path)
    }

    std::unique_ptr<proto::Writer> NodeGopherMap::make_writer(
            const proto::WriteParams& wp,
            const request::Request& request)
    {
        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        writer->text("test");

        return out;
    }

}
