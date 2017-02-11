#include "node-dirlist.h"
#include "proto.h"

#include <string>

namespace spg::gopher
{
    NodeDirList::NodeDirList(const Map &m,
                             const std::string& display_name,
                             const std::string& selector,
                             const std::string& host,
                             uint16_t port) :
        Node(
            NodeType::NT_DIRLIST,
            display_name,
            selector,
            host,
            port
        ),
        map(m)
    {
    }

    void NodeDirList::insert(const Node& item)
    {
        subs.push_back(item.selector);
    }

    void NodeDirList::show(int fd)
    {
        using spg::gopher::proto::writeln;

        auto end = subs.cend();
        auto i = subs.begin();

        while (i != end) {
            try {
                writeln(fd, map.lookup(*i).repr);
                i ++;
            }
            catch (LookupFailure &e) {
                i = subs.erase(i);
            }
        }
    }

}

