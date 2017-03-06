#pragma once

#include <list>
#include <utility>
#include <string>

namespace spg::util
{
    struct StrRef
    {
        StrRef(const std::string& s);
        StrRef(const char* s, size_t len);

        const char* const start;
        const size_t len;

        std::string as_string() const;
    };

    std::list<StrRef> tokenize(const std::string& str, char sep);

    template <typename T>
    T strto(const std::string& str);
}
