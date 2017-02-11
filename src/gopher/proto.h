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

    /* Minimal wrapper for unix's write: writes at most len bytes from
     * buffer, returns the actual amount of bytes written, throws IOError in
     * case of error.
     */
    size_t write(int fd, const void* buffer, size_t len);

    /* Stateful reader class */
    template <
        size_t Size,
        typename ListenerT,
        void (ListenerT::*GotLine)(const char *data, size_t size),
        void (ListenerT::*GotEof)()
    >
    class Reader
    {
        public:
            Reader(ListenerT& lst)
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

            ListenerT& listener;
            std::array<char, Size> buffer;
            size_t cursor;
    };
}
