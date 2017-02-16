#pragma once

#include <ostream>

#include "node.h"
#include "node-menu.h"

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
