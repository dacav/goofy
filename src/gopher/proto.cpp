#include "proto.h"

#include <unistd.h>
#include <cerrno>

namespace spg::gopher::proto
{
    size_t read(int fd, void* buffer, size_t len)
    {
        ssize_t n = ::read(fd, buffer, len);
        if (n < 0) {
            throw IOError("read", errno);
        }
        return size_t(n);
    }

    size_t write(int fd, const void* buffer, size_t len)
    {
        ssize_t n = write(fd, buffer, len);
        if (n < 0) {
            throw IOError("write", errno);
        }
        return size_t(n);
    }

}
