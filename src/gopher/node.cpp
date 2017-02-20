#include "node.h"
#include "proto.h"

namespace spg::gopher
{

    Node::Node(NodeType type,
               const std::string& display_name,
               const std::string& selector,
               const std::string& host,
               uint16_t port) :
        info(
            type,
            display_name,
            selector,
            host,
            port
        )
    {
        for (auto &s : {display_name, selector, host}) {
            if (s.find('\t') != std::string::npos) {
                throw NodeError(std::string("Unallowed tab sign: ") + s);
            }
        }
    }

    Node::~Node()
    {
    }

}
