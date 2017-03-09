#include "node-types.h"

namespace spg::gopher
{

    NodeInfo::NodeInfo(
            NodeType _type,
            std::string&& dn,
            std::string&& sl,
            std::string&& h,
            uint16_t p) :
        type(_type),
        display_name(std::move(dn)),
        selector(std::move(sl)),
        host(std::move(h)),
        port(p)
    {
    }

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
