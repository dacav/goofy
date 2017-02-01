#include "node-dirlist.h"

#include <string>

namespace spg::gopher
{
    NodeDirList::NodeDirList(const std::string& display_name,
                             const std::string& selector,
                             const std::string& host,
                             uint16_t port)
        : Node(
            NodeType::NT_DIRLIST,
            display_name,
            selector,
            host,
            port)
    {
    }

    const NodeDirList::NodeList NodeDirList::list_nodes() const
    {
        return nodes;
    }

    void NodeDirList::insert(const NodeLink& item)
    {
        nodes.push_back(item);
    }
}

