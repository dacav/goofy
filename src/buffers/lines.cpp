#include "lines.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>
#include <cerrno>
#include <cassert>
#include <tuple>
#include <sstream>

namespace spg::buffers
{
    Error::Error(const std::string &msg)
        : std::runtime_error(msg)
    {
    }

    Error::Error(const std::string &what, int err)
        : std::runtime_error(what + ": " + strerror(err))
    {
    }

    OverflowError::OverflowError(size_t skipping)
        : Error(std::string("Line buffer overflown, skipping ")
                + std::to_string(skipping) + " bytes")
    {
    }

    IOError::IOError(const std::string &what, int err)
        : Error(what, err)
    {
    }

    LineBuffer::LineBuffer(size_t buf_size)
        : bytes(buf_size)
        , leftover(0)
    {
    }

    const LineBuffer::LinesList& LineBuffer::read(int fd)
    {
        resume();

        ssize_t length = recv(
            fd,
            &bytes[leftover],
            bytes.size() - leftover,
            MSG_DONTWAIT
        );

        if (length < 0)
        {
            const int e = errno;
            if (e == EWOULDBLOCK || e == EAGAIN)
            {
                return lines;
            }
            throw IOError("recv", e);
        }
        if (length > 0)
        {
            parse(length);
        }
        return lines;
    }

    void LineBuffer::parse(size_t parse_len)
    {
        char* start = &bytes[0];
        parse_len += leftover;
        leftover = 0;
        while (parse_len > 0)
        {
            char* marker = (char*)std::memchr(start, '\n', parse_len);
            if (marker == nullptr)
            {
                /* We have leftovers, belonging to the next line. */
                leftover += parse_len;
                check_overflow();
                break;
            }
            else
            {
                /* We found a marker. */
                const size_t len = marker - start + 1;

                lines.emplace_back(start, len);

                start += len;
                parse_len -= len;
            }
        }
    }

    void LineBuffer::resume()
    {
        if (leftover > 0 && !lines.empty())
        {
            /* We have leftovers from previous read, and they are shifted as
             * recorded in the last element of the lines list. We bring them
             * to the beginning of the buffer.
             */
            const char *start;
            size_t last_len;

            std::tie(start, last_len) =  lines.back();
            std::copy(
                start + last_len,
                start + last_len + leftover,
                &bytes[0]
            );
        }
        lines.clear();
    }

    void LineBuffer::check_overflow()
    {
        /* Repair and throw error if we are out of buffer space. This might
         * happen if input stream produces a line long enough to fill up the
         * buffer.
         */
        assert(bytes.size() >= leftover);
        if (bytes.size() == leftover)
        {
            leftover = 0;
            lines.clear();
            throw OverflowError(bytes.size());
        }
    }

} // namespace buffers
