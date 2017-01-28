#include "node.h"

#include <sstream>

namespace spg::gopher
{

    Node::Node(NodeType _type,
               const std::string _display_name,
               const std::string _selector,
               const std::string _host,
               uint16_t _port)
        : type(_type)
        , display_name(_display_name)
        , selector(_selector)
        , host(_host)
        , port(_port)
    {
        for (auto &s : {display_name, selector, host}) {
            if (s.find('\t') != std::string::npos) {
                throw NodeError(std::string("Unallowed tab sign: ") + s);
            }
        }
    }

    NodeRepr::NodeRepr(const Node &n)
        : node(n)
    {
    }

}
