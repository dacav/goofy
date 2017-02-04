#pragma once

#include <cstddef>
#include <string>

#include "../error.h"

namespace spg::gopher::proto
{
    class IOError : public Error
    {
        public:
            IOError(const std::string &msg, int e)
                : Error(msg, e)
            {}
    };


    void write(int fd, const char* bytes, size_t len);
    void write(int fd, const std::string& str);

    void writeln(int fd, const char* bytes, size_t len);
    void writeln(int fd, const std::string& str);

    void writetb(int fd, const char* bytes, size_t len);
    void writetb(int fd, const std::string& str);
}
