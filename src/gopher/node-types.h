#pragma once

#include <string>
#include <cstdint>

#include "../util/typeguesser.h"

namespace goofy::gopher
{
    enum NodeType {
        NT_PLAIN = '0',
        NT_MENU = '1',
        NT_ERROR = '3',
        NT_BINARY = '9',
        NT_INFO = 'i',
        NT_GIF = 'g',
        NT_IMAGE = 'I',
        NT_HYPERTEXT = 'h',
    };

    class GopherTypeGuesser : public util::TypeGuesser
    {
        public:
            GopherTypeGuesser();

            NodeType type_of(const std::string& path) const;

        private:
            static bool matches(const char* got, const char* pattern);
            NodeType type_of_file(const std::string& path) const;
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
