#include "str.h"
#include "../error.h"

#include <algorithm>

namespace spg::util
{
    StrRef::StrRef() :
        StrRef(nullptr, 0)
    {}

    StrRef::StrRef(const StrRef& oth) :
        StrRef(oth.start, oth.len)
    {}

    StrRef::StrRef(const std::string& s) :
        StrRef(s.c_str(), s.length())
    {}

    StrRef::StrRef(const char* s, size_t l) :
        start(s),
        len(l)
    {}

    StrRef& StrRef::operator=(const StrRef& other)
    {
        start = other.start;
        len = other.len;
        return *this;
    }

    StrRef::operator std::string() const
    {
        return std::string(start, len);
    }

    bool StrRef::empty() const
    {
        return start == nullptr;
    }

    StrRef& StrRef::operator++(int)
    {
        start += 1;
        len --;
        return *this;
    }

    StrRef& StrRef::operator+=(int offs)
    {
        start += offs;
        len -= offs;
        return *this;
    }

    std::list<StrRef> tokenize(const StrRef& str, char sep)
    {
        size_t count = 0;
        for (unsigned i = 0; i < str.len; i ++) {
            if (str.start[i] == sep) count ++;
        }
        if (count == 0) {
            return {str};
        }

        std::list<StrRef> out;
        const char* cursor = str.start;
        size_t remains = str.len;
        while (count --) {
            const char* end = (const char*) memchr(
                (void *)cursor,
                sep,
                remains
            );
            const size_t len = end - cursor;

            out.emplace_back(cursor, len);
            cursor = end + 1;
            remains -= len + 1;
        }
        if (remains) {
            out.emplace_back(cursor, remains);
        }

        return out;
    }

    template <>
    uint16_t strto(const std::string& string)
    {
        char* end = nullptr;
        errno = 0;
        auto val = strtoul(string.data(), &end, 10);
        if (val == 0 && (errno != 0 || end == string.data())) {
            std::string err("Invalid uint16: '");
            err += string;
            err += '\'';
            throw Error(err, errno);
        }

        return val;
    }

}
