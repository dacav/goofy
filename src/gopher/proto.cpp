#include "proto.h"
#include "../error.h"

#include <unistd.h>
#include <cerrno>

namespace spg::gopher::proto
{
    void write(int fd, const char* bytes, size_t len)
    {
        while (len > 0) {
            ssize_t n = ::write(fd, bytes, len);
            if (n < 0) {
                throw spg::IOError("write", errno);
            }
            bytes += n;
            len -= n;
        }
    }

    void write(int fd, const std::string& str)
    {
        write(fd, str.c_str(), str.length());
    }

    void writeln(int fd, const char* bytes, size_t len)
    {
        write(fd, bytes, len);
        write(fd, "\r\n", 2);
    }

    void writeln(int fd, const std::string& str)
    {
        write(fd, str);
        write(fd, "\r\n", 2);
    }

    void writetb(int fd, const char* bytes, size_t len)
    {
        write(fd, bytes, len);
        write(fd, "\t", 1);
    }

    void writetb(int fd, const std::string& str)
    {
        write(fd, str);
        write(fd, "\t", 1);
    }
}
