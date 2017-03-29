#pragma once

#include <list>
#include <utility>
#include <limits>
#include <string>

#include "error.h"

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
    T strto(const std::string& str)
    {
        char* end = nullptr;
        errno = 0;
        auto val = strtoul(str.data(), &end, 10);
        if (val == 0 && (errno != 0 || end == str.data())) {
            throw Error("Invalid integer: '" + str + '\'', errno);
        }
        const auto max = std::numeric_limits<T>::max();
        const auto min = std::numeric_limits<T>::min();
        if (val > max || val < min) {
            throw Error("Value " + std::to_string(val)
                + " not in range ["
                + std::to_string(min) + ':' + std::to_string(max) + ']'
            );
        }

        return val;
    }
}
