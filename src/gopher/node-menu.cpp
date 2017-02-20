#include "node-menu.h"
#include "proto.h"

#include <string>

namespace spg::gopher
{
    NodeMenu::NodeMenu(const Map &m,
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

    void NodeMenu::insert(const Node& item)
    {
        subs.push_back(item.info.selector);
    }

    std::unique_ptr<gopher::proto::Writer> NodeMenu::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        using spg::gopher::proto::LinesWriter;

        auto writer = new LinesWriter(wp);
        std::unique_ptr<Writer> out(writer);
        auto end = subs.cend();
        auto i = subs.begin();

        while (i != end) {
            try {
                writer->insert(map.lookup(*i).repr);
                i ++;
            }
            catch (LookupFailure &e) {
                i = subs.erase(i);
            }
        }

        return out;
    }

}

