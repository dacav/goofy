#pragma once

#include <cstddef>
#include <string>
#include <cstring>
#include <array>

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

    /* Stateful reader class */
    template <
        size_t Size,
        typename Listener,
        void (Listener::*GotLine)(const char *data, size_t size),
        void (Listener::*GotEof)()
    >
    class Reader
    {
        public:
            Reader(Listener& lst)
                : listener(lst)
                , cursor(0)
            {
            }

            void read(int fd)
            {
                const size_t room = buffer.size() - cursor;
                char* start = &buffer[cursor];

                size_t size = spg::gopher::proto::read(fd, start, room);

                if (size == 0) {
                    if (cursor > 0) {
                        (listener.*GotLine)(&buffer[0], cursor);
                    }
                    (listener.*GotEof)();
                    reset();
                    return;
                }

                cursor += size;
                char* newline = reinterpret_cast<char *>(
                    std::memchr(start, '\n', room - size)
                );

                if (newline != nullptr) {
                    if (newline > &buffer[0] && newline[-1] == '\r') {
                        newline --;
                    }
                    *newline = '\0';
                    (listener.*GotLine)(&buffer[0], newline - &buffer[0]);
                    reset();
                }
            }

        private:
            inline void reset()
            {
                cursor = 0;
            }

            Listener& listener;
            std::array<char, Size> buffer;
            size_t cursor;
    };

    void write(int fd, const char* bytes, size_t len);
    void write(int fd, const std::string& str);

    void writeln(int fd, const char* bytes, size_t len);
    void writeln(int fd, const std::string& str);

    void writedone(int fd);
}
