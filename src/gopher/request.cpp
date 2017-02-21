#include "request.h"

#include <cstring>

namespace
{
    const std::string ROOT_SELECTOR("");

    std::vector<std::string> split_path(const char* begin, size_t len)
    {
        std::vector<std::string> out;

        while (len > 0) {
            const char* end = (const char*) memchr(begin, '/', len);

            if (end == nullptr) {
                out.emplace_back(begin, len);
                out.back().shrink_to_fit();
                len = 0;
            }
            else {
                const size_t step_len = end - begin;
                if (step_len > 0) {
                    out.emplace_back(begin, step_len);
                    out.back().shrink_to_fit();
                }

                len -= step_len + 1;
                begin += step_len + 1;
            }
        }
        out.shrink_to_fit();
        return out;
    }
}

namespace spg::gopher::request
{
    Request::Request(const char* line, size_t len) :
        query(split_path(line, len)),
        selector(query.size() > 0 ? query.front() : ROOT_SELECTOR)
    {
    }
}
