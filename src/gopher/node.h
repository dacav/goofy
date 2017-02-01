#pragma once

#include <string>

#include "../error.h"

namespace spg::gopher
{
    enum NodeType {
        NT_PLAIN = '0',
        NT_DIRLIST = '1',
    };

    class NodeError : public spg::Error {
        public:
            NodeError(const std::string msg) : spg::Error(msg) {}
    };

    class Node {
        public:
            Node(NodeType type,
                 const std::string& display_name,
                 const std::string& selector,
                 const std::string& host,
                 uint16_t port);

            const char type;
            const std::string display_name;
            const std::string selector;
            const std::string host;
            const uint16_t port;
    };

    // Representation of a node for gopher protocol
    //
    // Simple abuse of the operator<< of ostream classes for yielding a
    // string representation suitable for gopher
    struct NodeRepr {
        NodeRepr(const Node &node);
        const Node &node;
    };

}

#include <ostream>

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
        << node.port << "])"
    ;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const spg::gopher::NodeRepr &r)
{
    return os
        << r.node.type << r.node.display_name
        << '\t' << r.node.selector
        << '\t' << r.node.host
        << '\t' << r.node.port
        << "\r\n";
    ;
}
