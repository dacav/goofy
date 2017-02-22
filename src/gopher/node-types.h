#pragma once

#include <string>
#include <cstdint>

namespace spg::gopher
{
    enum NodeType {
        NT_PLAIN = '0',
        NT_MENU = '1',
        NT_ERROR = '3',
        NT_INFO = 'i',
    };

    struct NodeInfo {
        NodeInfo(
            NodeType type,
            const std::string& display_name,
            const std::string& selector,
            const std::string& host,
            uint16_t port
        );

        const char type;
        const std::string display_name;
        const std::string selector;
        const std::string host;
        const uint16_t port;

        std::string mkrepr() const;
    };
}
