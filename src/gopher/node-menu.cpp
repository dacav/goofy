#include "node-menu.h"
#include "proto.h"

#include <string>

namespace spg::gopher
{
    NodeMenu::NodeMenu(
            const settings::Settings &sets,
            const std::string& display_name,
            const std::string& selector,
            const std::string& host,
            uint16_t port) :
        Node(
            NodeType::NT_MENU,
            display_name,
            selector,
            host,
            port
        ),
        settings(sets)
    {
    }

    void NodeMenu::insert(const NodeInfo& info)
    {
        subs.push_back(info);
    }

    void NodeMenu::insert(const Node& node)
    {
        insert(node.info);
    }

    std::unique_ptr<gopher::proto::Writer> NodeMenu::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        using spg::gopher::proto::MenuWriter;

        auto writer = new MenuWriter(wp);
        std::unique_ptr<Writer> out(writer);

        for (const NodeInfo& info : subs) {
            writer->node(info);
        }

        return out;
    }

}

