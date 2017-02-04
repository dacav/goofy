#include "proto.h"

#include <unistd.h>
#include <cerrno>

namespace spg::gopher::proto
{
    void write(int fd, const char* bytes, size_t len)
    {
        while (len > 0) {
            ssize_t n = ::write(fd, bytes, len);
            if (n < 0) {
                throw IOError("write", errno);
            }
            bytes += n;
            len -= n;
        }
    }

    size_t read(int fd, void *buffer, size_t len)
    {
        ssize_t n = ::read(fd, buffer, len);
        if (n < 0) {
            throw IOError("read", errno);
        }
        return size_t(n);
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

    void writedone(int fd)
    {
        write(fd, "\r\n.\r\n", 5);
    }
}
