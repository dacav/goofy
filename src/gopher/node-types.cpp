#include "node-types.h"

#include <cstring>

namespace goofy::gopher
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

    GopherTypeGuesser::GopherTypeGuesser() :
        TypeGuesser()
    {
    }

    NodeType GopherTypeGuesser::type_of(const std::string& path) const
    {
        mode_t mode = util::mode_of(path);
        switch (mode & S_IFMT) {
            case S_IFREG:
                return type_of_file(path);
            case S_IFDIR:
                return gopher::NodeType::NT_MENU;
            default:
                return gopher::NodeType::NT_ERROR;
        }
    }

    bool GopherTypeGuesser::matches(const char* got, const char* pattern)
    {
        return strstr(got, pattern) == got;
    }

    NodeType GopherTypeGuesser::type_of_file(const std::string& path) const
    {
        const char* magic_guess = guess(path);

        if (matches(magic_guess, "text/")) {
            return gopher::NodeType::NT_PLAIN;
        }
        if (matches(magic_guess, "image/gif")) {
            return gopher::NodeType::NT_GIF;
        }
        if (matches(magic_guess, "image/")) {
            return gopher::NodeType::NT_IMAGE;
        }

        return gopher::NodeType::NT_BINARY;
    }

}
