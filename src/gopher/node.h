#pragma once

#include <cstddef>
#include <string>
#include <ostream>

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

            virtual void repr(int fd) const;
            virtual void show(int fd) const = 0;
    };

}
