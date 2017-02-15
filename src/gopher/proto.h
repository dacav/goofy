#pragma once

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <exception>

#include "../error.h"

#include <event2/event.h>

namespace spg::gopher::proto
{
    class IOError : public Error
    {
        public:
            IOError(const std::string &msg)
                : Error(msg)
            {}
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

    /* Unique_ptr based RAII wrapper around a 'struct event' from libevent */
    using Event = std::unique_ptr<
        struct event,
        void(*)(struct event *)
    >;

    /* Set of asynchornous callbacks and parameters for line-based reading */
    struct ReadParams {
        struct event_base* const ev_base;
        const timeval timeout;
        const std::function<bool(const char*, size_t)> got_line;
        const std::function<void(void)> got_eof;
        const std::function<void(void)> got_timeout;
        const std::function<void(std::exception&)> got_error;
    };

    /* Stateful reader class */
    class Reader
    {
        public:
            Reader(const ReadParams& params, size_t buflen);
            void read_from(int sock);

        private:
            const ReadParams read_params;
            std::vector<char> buffer;
            size_t cursor;
            spg::gopher::proto::Event ev_read;

            static void cb_read(int sock, short what, void *arg);
            void next();
            void read_chunk(int sock);
            void reset();
    };

    /* Set of asynchornous callbacks and parameters for line-based reading */
    struct WriteParams {
        struct event_base* const ev_base;
        const timeval timeout;
        const std::function<void(void)> got_success;
        const std::function<void(std::exception&)> got_error;
        const std::function<void(void)> got_timeout;
    };

    /* Stateful writer class */
    class Writer
    {
        public:
            Writer(const WriteParams& params);
            virtual ~Writer();

            void write_to(int sock);

        protected:
            virtual void write_chunk(int sock) = 0;
            virtual void before_write();
            void next();
            const WriteParams write_params;

        private:
            spg::gopher::proto::Event ev_write;
            virtual void end();

            static void cb_write(int sock, short what, void *arg);
    };

    class LinesWriter : public Writer
    {
        public:
            LinesWriter(const WriteParams& params);
            void insert(const std::string& line);
            virtual void before_write() override;

        private:
            std::vector<char> buffer;
            unsigned cursor;

            virtual void write_chunk(int sock) override;
    };

}
