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

    Reader::Reader(const ReadParams& rp,
                   size_t buflen) :
        read_params(rp),
        buffer(buflen),
        cursor(0),
        ev_read(nullptr, event_free)
    {
    }

    void Reader::read_from(int sock)
    {
        assert(ev_read.get() == nullptr);

        ev_read.reset(event_new(
            read_params.ev_base,
            sock,
            EV_READ | EV_TIMEOUT,
            Reader::cb_read,
            this
        ));

        // TODO: Just not mentioned by the reference of libevent.
        // Try putting -1, see if this is really true.
        assert(ev_read.get() != nullptr);
        next();
    }

    void Reader::cb_read(int sock, short what, void *arg)
    {
        Reader& reader = *reinterpret_cast<Reader*>(arg);

        assert(!(what & (EV_WRITE | EV_SIGNAL)));
        if (what & EV_READ) {
            try {
                reader.read_chunk(sock);
            }
            catch (std::exception& e) {
                reader.read_params.got_error(e);
                reader.reset();
            }
        }
        else if (what & EV_TIMEOUT) {
            reader.read_params.got_timeout();
            reader.reset();
        }
    }

    void Reader::next()
    {
        if (event_add(ev_read.get(), &read_params.timeout) == -1) {
            IOError("event_add (EV_READ|EV_TIMEOUT)", errno);
        }
    }

    void Reader::read_chunk(int sock)
    {
        const size_t room = buffer.size() - cursor;
        char* start = &buffer[cursor];

        size_t size = spg::gopher::proto::read(sock, start, room);

        if (size == 0) {
            read_params.got_eof();
            reset();
            return;
        }

        cursor += size;
        char* newline = reinterpret_cast<char *>(
            std::memchr(start, '\n', room - size)
        );

        bool again = true;

        if (newline != nullptr) {
            if (newline > &buffer[0] && newline[-1] == '\r') {
                newline --;
            }
            *newline = '\0';
            again = read_params.got_line(
                &buffer[0],
                newline - &buffer[0]
            );
            reset();
        }

        if (again) {
            next();
        }
    }

    void Reader::reset()
    {
        cursor = 0;
        ev_read.reset();
    }

    Writer::Writer(const WriteParams& params) :
        write_params(params),
        ev_write(nullptr, event_free)
    {
    }

    void Writer::write_to(int sock)
    {
        assert(ev_write.get() == nullptr);

        ev_write.reset(event_new(
            write_params.ev_base,
            sock,
            EV_WRITE | EV_TIMEOUT,
            Writer::cb_write,
            this
        ));

        // TODO: Just not mentioned by the reference of libevent.
        // Try putting -1, see if this is really true.
        assert(ev_write.get() != nullptr);
        next();
    }

    void Writer::cb_write(int sock, short what, void *arg)
    {
        Writer& writer = *reinterpret_cast<Writer*>(arg);

        assert(!(what & (EV_READ | EV_SIGNAL)));
        if (what & EV_WRITE) {
            try {
                writer.write_chunk(sock);
            }
            catch (std::exception& e) {
                writer.write_params.got_error(e);
                writer.reset();
            }
        }
        else if (what & EV_TIMEOUT) {
            writer.write_params.got_timeout();
            writer.reset();
        }
    }

    void Writer::next()
    {
        if (event_add(ev_write.get(), &write_params.timeout) == -1) {
            // TODO: beware of throws
            throw IOError("event_add (EV_READ|EV_TIMEOUT)", errno);
        }
    }

    void Writer::reset()
    {
        ev_write.reset();
    }

}
