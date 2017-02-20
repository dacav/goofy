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

}
