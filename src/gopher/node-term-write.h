#pragma once

#include <ostream>

#include "node.h"
#include "node-dirlist.h"

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const spg::gopher::Node &node)
{
    return os
        << "spg::gopher::Node(["
        << node.type << "], ["
        << node.display_name << "], ["
        << node.selector << "], ["
        << node.host << "], ["
        << node.port << "])";
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const spg::gopher::ProtoRepr &pd)
{
    return os
        << pd.node.type << pd.node.display_name
        << '\t' << pd.node.selector
        << '\t' << pd.node.host
        << '\t' << pd.node.port
        << "\r\n";
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const spg::gopher::ProtoShow &ps)
{
    using namespace spg::gopher;

    switch (ps.node.type)
    {
        case NT_PLAIN:
            break;

        case NT_DIRLIST:
            for (auto& sub : reinterpret_cast<const NodeDirList &>(ps.node).list_nodes()) {
                os << ProtoRepr(*sub);
            }

    }
    return os;
}
