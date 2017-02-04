#pragma once

#include <list>
#include <vector>
#include <stdexcept>
#include <utility>

#include <cstddef>

namespace spg::buffers
{
    class Error : public std::runtime_error {
        public:
            Error(const std::string &msg);
            Error(const std::string &what, int err);
    };

    class OverflowError : public Error {
        public:
            OverflowError(size_t skipping);
    };

    class IOError : public Error {
        public:
            IOError(const std::string &what, int err);
    };

    /* Split the incoming data of a socket with a line-based semantics.
     *
     * The line separation is assuming UNIX-style new lines markers ('\n')
     * over a stream of ASCII or UTF-8 text.
     *
     * Reads from the socket are non blocking.
     */
    class LineBuffer {
        public:
            /* Constructor for the buffer.
             *
             * Requires as parameter the buffer size, which determins how
             * big the reading chunk is.
             *
             * The second parameter defines what character shoud replace the
             * new-line marker. It defaults to '\0' for convenience in
             * string construction, it could be '\n' if one wants to keep
             * the new line terminators.
             */
            LineBuffer(size_t bufSize);

            /* Read a list of lines the socket descriptor
             *
             * The returned list of lines is empty if no complete line was
             * read (that is, if nothing was received on the socket or if no
             * new-line marker was found in the received data).
             *
             * Each returned line is a pair: the first element is a pointer
             * to the beginning of the line, and the second is the size of
             * the line. The new line marker is replaced with a marker
             * replacement (as provided in the constructor), and the size is
             * inclusive of such character.  The returned list remains valid
             * until the subsequent call of this method or the destruction
             * of the object.
             *
             * Throws line_buffering::IOError on read error.
             *
             * Throws line_buffering::OverflowError if the buffer was
             * clogged (when this happens, a chunk of data gets discarded,
             * but the LineBuffer object is ready to read the next chunk.
             */
            using LinesList = std::list<std::pair<const char*, size_t>>;
            const LinesList& read(int sd);

        private:
            std::vector<char> bytes;
            LinesList lines;
            unsigned leftover;

            void resume();
            void check_overflow();
            void parse(size_t parse_len);
    };

} // namespace line_buffering
