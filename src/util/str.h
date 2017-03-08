#pragma once

#include <list>
#include <utility>
#include <string>

namespace spg::util
{
    struct StrRef
    {
        StrRef(const std::string& s, size_t offs=0);
        StrRef(const std::string& s, size_t offs, size_t len);
        StrRef(const char* s, size_t offs, size_t len);

        const char* const start;
        const size_t len;
    };

    std::list<StrRef> tokenize(const std::string& str, char sep);

    template <typename T>
    T strto(const std::string& str);
}
