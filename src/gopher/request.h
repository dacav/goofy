#pragma once

#include <cstddef>
#include <string>
#include <list>

#include "../util/str.h"

namespace spg::gopher::request
{
    class Request
    {
        public:
            Request(const char* selector, size_t len);

            std::string selector() const;
            std::list<util::StrRef> query() const;
            std::string as_path() const;
            std::string url() const;

            const std::string raw_body;
            const bool is_url;

        private:
            const char separator;
            const size_t first;
    };

}
