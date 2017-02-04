#include "node.h"
#include "proto.h"

namespace
{
    std::string mkrepr(spg::gopher::NodeType type,
                       const std::string& display_name,
                       const std::string& selector,
                       const std::string& host,
                       uint16_t port)
    {
        std::string portstr = std::to_string(port);

        std::string out;
        out.reserve(
            1 + display_name.length() +
            1 + selector.length() +
            1 + host.length() +
            1 + portstr.length()
        );

        out += char(type);
        out += display_name;
        out += '\t';
        out += selector;
        out += '\t';
        out += host;
        out += '\t';
        out += portstr;
        return out;
    }

}

namespace spg::gopher
{

    Node::Node(NodeType _type,
               const std::string& _display_name,
               const std::string& _selector,
               const std::string& _host,
               uint16_t _port)
        : type(_type)
        , display_name(_display_name)
        , selector(_selector)
        , host(_host)
        , port(_port)
        , repr(mkrepr(_type, display_name, selector, host, port))
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
