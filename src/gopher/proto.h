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
#include <initializer_list>

#include "../error.h"
#include "node-types.h"

#include <event2/event.h>
#include <sys/sendfile.h>

namespace spg::gopher::proto
{
    /* Minimal wrapper for unix's read: reads at most len bytes into buffer,
     * returns the actual amount of bytes read, throws IOError in case of error.
     */
    size_t read(int fd, void *buffer, size_t len);

    /* Minimal wrapper for unix's write: writes at most len bytes from
     * buffer, returns the actual amount of bytes written, throws IOError in
     * case of error.
     */
    size_t write(int fd, const void* buffer, size_t len);

    /* The same as spg::gopher::proto::write, but uses a MSG_DONTWAIT flag
     * in order to have a non-blocking send.
     *
     * The EWOULDBLOCK condition triggers an IOError exception as any other
     * error condition. It can be distinguished from other errors by means
     * of the 'errno_was' field of the IOError class.
     */
    size_t write_nonblock(int fd, const void* buffer, size_t len);

    /* Unique_ptr based RAII wrapper around a 'struct event' from libevent */
    using Event = std::unique_ptr<
        struct event,
        void(*)(struct event *)
    >;

    /* Set of asynchornous callbacks and parameters for line-based reading */
    struct ReadParams {
        /* Libevent's "event base" object */
        struct event_base* const ev_base;

        /* ReadParams::got_line gets called if a whole line is
         * received. If the callback returns true another line is read from
         * the socket;
         */
        const std::function<bool(const char*, size_t)> got_line;

        /* ReadParams::got_eof gets called if the peer closed the
         * client-to-server side of the communication (so no more data can
         * be read);
         */
        const std::function<void(void)> got_eof;

        /* ReadParams::got_timeout gets called after a certain inactivity
         * period (determined by the ReadParams::timeout parameter);
         */
        const timeval timeout;
        const std::function<void(void)> got_timeout;

        /* ReadParams::got_error gets called upon error from an underlying
         * system (libevent or the operating system). It is parametrized
         * with an exception representing the error.
         */
        const std::function<void(std::exception&)> got_error;
    };

    /* Stateful asynchronous reader class */
    class Reader
    {
        public:
            /* The constructor accepts the reading parameters and the size
             * of the internal read buffer. The size must be able to cover
             * the maximum expected line.
             */
            Reader(const ReadParams& params, size_t buflen);

            /* Start reading from a given socket descriptor. This will
             * eventually trigger one of the functions of the ReadParams
             * structure passed as parameter.
             *
             * A read is terminated when one of the following happens:
             * - ReadParams::got_eof is invoked;
             * - ReadParams::got_timeout is invoked;
             * - ReadParams::got_error is invoked;
             * - ReadParams::got_line is invoked and it returns false.
             *
             * After a terminated read, the Reader::read_from method can be
             * invoked again, possibly with another socket descriptor.
             * Invoking this method for a non-terminated read will throw
             * an IOError.
             */
            void read_from(int sock);

        private:
            const ReadParams read_params;
            std::vector<char> buffer;
            size_t cursor;
            spg::gopher::proto::Event ev_read;

            static void cb_read(int sock, short what, void *arg);
            void schedule();
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
            void schedule();
            const WriteParams write_params;

        private:
            spg::gopher::proto::Event ev_write;
            virtual void end();

            static void cb_write(int sock, short what, void *arg);
    };

    class MenuWriter : public Writer
    {
        public:
            MenuWriter(const WriteParams& params);
            void insert(const NodeInfo& info);
            virtual void before_write() override;

        protected:
            inline void append(const char* bytes, size_t len);
            inline void append(const std::string& string);
            inline void append(const char& byte);

        private:
            std::vector<char> buffer;
            unsigned cursor;

            virtual void write_chunk(int sock) override;
    };

    class FileWriter : public Writer
    {
        public:
            FileWriter(const WriteParams& params, int fd);
            ~FileWriter();

        private:
            int fdesc;
            off_t offset;
            size_t to_send;

            static size_t file_size_bytes(int fd);
            virtual void write_chunk(int sock) override;
    };

    class ErrorWriter : public MenuWriter
    {
        public:
            ErrorWriter(const WriteParams& params, const UserError& e);
    };

}
