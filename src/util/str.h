#pragma once

#include <list>
#include <utility>
#include <string>

namespace spg::util
{
    struct StrRef
    {
        StrRef();
        StrRef(const std::string&);
        StrRef(std::string&&) = delete;
        StrRef(const char*, size_t);
        StrRef(const StrRef&);

        const char* const start;
        const size_t len;

        operator std::string() const;
        operator bool() const;
    };

    std::list<StrRef> tokenize(const StrRef&, char sep);

    template <typename T>
    T strto(const std::string& str);
}
