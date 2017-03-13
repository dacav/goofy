#include "node-types.h"

namespace spg::gopher
{

    NodeInfo::NodeInfo(
            NodeType _type,
            const std::string& dn,
            const std::string& sl,
            const std::string& h,
            uint16_t p) :
        type(_type),
        display_name(dn),
        selector(sl),
        host(h),
        port(p)
    {
    }

}
