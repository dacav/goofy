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
        size_t len = 0;
        const long long max = std::numeric_limits<T>::max();
        const long long min = std::numeric_limits<T>::min();

        try {
            const long long val = std::stoll(str, &len);

            if (val < min || val > max) {
                throw Error("Value '" + str + "' out of range [" +
                    std::to_string(min) + ':' + std::to_string(max) + ']'
                );
            }

            return T(val);
        }
        catch (std::out_of_range& e) {
            throw Error("Value '" + str + "' out of range [" +
                std::to_string(min) + ':' + std::to_string(max) + ']'
            );
        }

    }

    template <> uint64_t strto(const std::string& str);

}
