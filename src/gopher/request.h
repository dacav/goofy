#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace spg::gopher::request
{
    class Request
    {
        public:
            Request(const char* selector, size_t len);

            const std::vector<std::string> path;
            const std::string& selector;
    };

}
