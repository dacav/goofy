#include "proto.h"
#include "node-types.h"

#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

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
        ssize_t n = ::write(fd, buffer, len);
        if (n < 0) {
            throw IOError("write", errno);
        }
        return size_t(n);
    }

    size_t write_nonblock(int fd, const void* buffer, size_t len)
    {
        ssize_t n = ::send(fd, buffer, len, MSG_DONTWAIT);
        if (n < 0) {
            throw IOError("send", errno);
        }
        return size_t(n);
    }

    Reader::Reader(const ReadParams& rp,
                   size_t buflen) :
        read_params(rp),
        buffer(buflen + 2), // accounting for "\r\n"
        cursor(0),
        ev_read(nullptr, event_free)
    {
    }

    void Reader::read_from(int sock)
    {
        if (ev_read.get() != nullptr) {
            throw IOError("Reader::read_from of a busy reader");
        }

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
        schedule();
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
                reader.end();
                reader.read_params.got_error(e);
            }
        }
        else if (what & EV_TIMEOUT) {
            reader.end();
            reader.read_params.got_timeout();
        }
    }

    void Reader::schedule()
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
            end();
            read_params.got_eof();
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
            end();
        }

        if (again) {
            schedule();
        }
    }

    void Reader::end()
    {
        cursor = 0;
        ev_read.reset();
    }

    Writer::Writer(const WriteParams& params) :
        write_params(params),
        ev_write(nullptr, event_free)
    {
    }

    Writer::~Writer()
    {
    }

    void Writer::write_to(int sock)
    {
        assert(ev_write.get() == nullptr);

        before_write();
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
        schedule();
    }

    void Writer::before_write()
    {
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
                writer.end();
                writer.write_params.got_error(e);
            }
        }
        else if (what & EV_TIMEOUT) {
            writer.end();
            writer.write_params.got_timeout();
        }
    }

    // TODO: rename as 'next_event' or 'wait_ready' or so...
    void Writer::schedule()
    {
        if (event_add(ev_write.get(), &write_params.timeout) == -1) {
            // TODO: beware of throws
            throw IOError("event_add (EV_READ|EV_TIMEOUT)", errno);
        }
    }

    void Writer::end()
    {
        ev_write.reset();
    }

    MenuWriter::MenuWriter(const WriteParams& params) :
        Writer(params),
        cursor(0)
    {
    }

    void MenuWriter::node(const NodeInfo& info)
    {
        append(info.type);
        append(info.display_name);
        append('\t');
        append(info.selector);
        append('\t');
        append(info.host);
        append('\t');
        append(std::to_string(info.port));
        append('\r');
        append('\n');
    }

    void MenuWriter::text(const char* msg, size_t len)
    {
        const char eol[] = "\t\tinfo.host\t1\r\n";
        append(char(NodeType::NT_INFO));
        append(msg, len > 0 ? len : std::strlen(msg));
        append(eol, sizeof(eol) - 1);
    }

    void MenuWriter::text(const std::string& msg)
    {
        text(msg.c_str(), msg.length());
    }

    void MenuWriter::error(const char* msg, size_t len)
    {
        const char eol[] = "\t\terror.host\t1\r\n";
        append(char(NodeType::NT_ERROR));
        append(msg, len > 0 ? len : std::strlen(msg));
        append(eol, sizeof(eol) - 1);
    }

    void MenuWriter::error(const std::string& msg)
    {
        error(msg.c_str(), msg.length());
    }

    void MenuWriter::before_write()
    {
        Writer::before_write();

        const char line[] = ".\r\n";
        append(line, sizeof(line) - 1);
        buffer.shrink_to_fit();
    }

    void MenuWriter::append(const char* bytes, size_t len)
    {
        buffer.insert(buffer.end(), bytes, bytes + len);
    }

    void MenuWriter::append(const std::string& string)
    {
        buffer.insert(buffer.end(), string.begin(), string.end());
    }

    void MenuWriter::append(const char& byte)
    {
        buffer.push_back(byte);
    }

    void MenuWriter::write_chunk(int sock)
    {
        size_t sent = spg::gopher::proto::write(sock,
            &buffer[cursor], buffer.size() - cursor
        );

        if (sent == 0) {
            throw IOError("wrote 0 bytes");
        }

        cursor += sent;
        if (cursor < buffer.size()) {
            schedule();
        }
        else {
            assert(cursor == buffer.size()); // never >
            write_params.got_success();
        }
    }

    FileWriter::FileWriter(
            const WriteParams& params,
            int fd) :
        Writer(params),
        fdesc(fd),
        offset(0),
        to_send(file_size_bytes(fd))
    {
    }

    void FileWriter::write_chunk(int sock)
    {
        ssize_t sent = sendfile(sock, fdesc, &offset, to_send);
        if (sent == -1) {
            throw IOError("sendfile", errno);
        }
        to_send -= sent;
        if (to_send > 0) {
            schedule();
        }
        else {
            write_params.got_success();
        }
    }

    FileWriter::~FileWriter()
    {
        close(fdesc);
    }

    size_t FileWriter::file_size_bytes(int fd)
    {
        struct stat statbuf;
        if (fstat(fd, &statbuf) == -1) {
            throw IOError("fstat", errno);
        }
        return statbuf.st_size;
    }
}
