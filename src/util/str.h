#pragma once

#include <list>
#include <utility>
#include <string>

namespace goofy::util
{
    struct StrRef
    {
        StrRef();
        StrRef(const std::string&);
        StrRef(std::string&&) = delete;
        StrRef(const char*, size_t);
        StrRef(const StrRef&);

        StrRef& operator++(int);
        StrRef& operator+=(int offs);
        StrRef& operator=(const StrRef&);

        const char* start;
        size_t len;
        bool empty() const;

        // Trims the left part of the string, removing spaces.
        // (note: rtrim is not implemented, since we treat the underlying
        // string as constant, and we cannot write a null-terminator.
        void ltrim();

        operator std::string() const;
    };

    std::list<StrRef> tokenize(const StrRef&, char sep);

    template <typename T>
    T strto(const std::string& str);
}
