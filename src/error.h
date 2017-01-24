#pragma once

#include <stdexcept>
#include <string>
#include <cstring>
#include <cerrno>

namespace spg
{

    class Error : public std::runtime_error
    {
        public:
            Error(const std::string &msg)
                : std::runtime_error(msg)
            {}
            Error(const std::string &when, int e)
                : std::runtime_error(when + ": " + std::strerror(e))
            {}
    };

}
