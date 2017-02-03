#include "node-dirlist.h"
#include "proto.h"

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

    const std::list<std::shared_ptr<Node>>& NodeDirList::list_nodes() const
    {
        return nodes;
    }

    void NodeDirList::insert(const std::shared_ptr<Node>& item)
    {
        nodes.push_back(item);
    }

    void NodeDirList::show(int fd) const
    {
        for (auto& sub : nodes) {
            sub->repr(fd);
        }
    }

}

