#pragma once

#include <iostream> // possibly dropme

#include <event2/event.h>
#include <memory>

namespace spg::session
{

    class Session
    {
        public:
            Session(struct event_base*, unsigned session_id, int clsock);
            const unsigned session_id;

            Session(Session&&);
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
    };

} // namespace session
