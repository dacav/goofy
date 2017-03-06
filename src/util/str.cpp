#include "str.h"
#include "../error.h"

#include <algorithm>

namespace spg::util
{
    StrRef::StrRef(const char* s, size_t l) :
        start(s),
        len(l)
    {}

    std::string StrRef::as_string() const
    {
        return std::string(start, len);
    }

    std::list<StrRef> tokenize(const std::string& str, char sep)
    {
        size_t count = 0;
        for (const char& c : str) {
            if (c == sep) count ++;
        }
        if (count == 0) {
            return {StrRef(str.c_str(), str.length())};
        }

        std::list<StrRef> out;
        size_t offs = 0;
        while (count --) {
            const size_t start = offs;
            offs = str.find_first_of(sep, start);
            const size_t len = offs - start;

            out.emplace_back(str.c_str() + start, len);
            offs += 1; // skip sep
        }
        out.emplace_back(str.c_str() + offs, str.length() - offs);

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
