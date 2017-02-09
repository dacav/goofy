#pragma once

#include <event2/event.h>
#include <memory>
#include <vector>
#include <cstddef>

#include "gopher/proto.h"

namespace spg::session
{
    class Session
    {
        public:
            Session(struct event_base*, unsigned session_id, int clsock);
            const unsigned session_id;

            Session(Session&&) = delete;
            Session(const Session&) = delete;
            void operator=(const Session&) = delete;
            void operator=(Session&&) = delete;

        private:
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

            gopher::proto::Reader<
                256ul,    // bytes
                Session,
                &Session::got_line,
                &Session::got_eof
            > reader;
    };

} // namespace session
