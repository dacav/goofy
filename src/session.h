#pragma once

#include <event2/event.h>
#include <memory>
#include <vector>
#include <cstddef>
#include <functional>

#include "gopher/proto.h"
#include "gopher/map.h"
#include "error.h"

namespace spg::session
{
    class SessionError : public spg::Error
    {
        public:
            SessionError(const std::string &msg)
                : spg::Error(msg)
            {}
            SessionError(const std::string &when, int e)
                : spg::Error(when, e)
            {}
    };

    class Session
    {
        public:
            using DropCallback = std::function<void()>;

            Session(
                spg::gopher::Map& gopher_map,
                const DropCallback& drop_callback,
                int clsock,
                struct event_base*
            );

            Session(Session&&) = delete;
            Session(const Session&) = delete;
            void operator=(const Session&) = delete;
            void operator=(Session&&) = delete;

        private:
            spg::gopher::Map& gopher_map;
            DropCallback drop_callback;
            int clsock;

            using Event = std::unique_ptr<
                struct event,
                void(*)(struct event *)
            >;
            Event ev_read;
            Event ev_write;

            static void cb_read(int clsock, short what, void *arg);
            static void cb_write(int clsock, short what, void *arg);

            void got_line(const char *line, size_t len);
            void got_eof();

            void close();

            gopher::proto::Reader<
                256ul,    // bytes
                Session,
                &Session::got_line,
                &Session::got_eof
            > reader;
    };

} // namespace session
