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

    /* Minimal wrapper for unix's read: reads at most len bytes into buffer,
     * returns the actual amount of bytes read, throws IOError in case of error.
     */
    size_t read(int fd, void *buffer, size_t len);

    void write(int fd, const char* bytes, size_t len);
    void write(int fd, const std::string& str);

    void writeln(int fd, const char* bytes, size_t len);
    void writeln(int fd, const std::string& str);

    void writedone(int fd);
}
