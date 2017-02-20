#include "node-types.h"

namespace spg::gopher
{

    NodeInfo::NodeInfo(
            NodeType _type,
            const std::string& _display_name,
            const std::string& _selector,
            const std::string& _host,
            uint16_t _port) :
        type(_type),
        display_name(_display_name),
        selector(_selector),
        host(_host),
        port(_port)
    {
    }

    std::string NodeInfo::mkrepr() const
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
