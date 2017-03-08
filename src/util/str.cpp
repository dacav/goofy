#include "str.h"
#include "../error.h"

namespace spg::util
{
    StrRef::StrRef(const std::string& s, size_t offs) :
        StrRef(s.c_str(), 0, s.length())
    {}

    StrRef::StrRef(const std::string& s, size_t offs, size_t len) :
        StrRef(s.c_str(), offs, len)
    {}

    StrRef::StrRef(const char* s, size_t offs, size_t l) :
        start(s + offs),
        len(l)
    {}

    std::list<StrRef> tokenize(const std::string& str, char sep)
    {
        size_t count = 0;
        for (const char& c : str) {
            if (c == sep) count ++;
        }
        if (count == 0) {
            return {StrRef(str)};
        }

        std::list<StrRef> out;
        size_t offs = 0;
        while (count --) {
            const size_t start = offs;
            offs = str.find_first_of(sep, start);
            const size_t len = offs - start;

            out.emplace_back(str, start, len);
            offs += 1; // skip sep
        }
        out.emplace_back(str, offs, str.length() - offs);

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
