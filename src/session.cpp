#include "session.h"

#include <cerrno>
#include <cstring>
#include <cassert>

#include <iostream>

namespace spg::session
{

    Session::Session(
            struct event_base *base_event,
            unsigned sid,
            int clsock) :
        session_id(sid),
        ev_read(
            event_new(
                base_event,
                clsock,
                EV_READ | EV_TIMEOUT,
                Session::cb_read,
                this
            ),
            event_free
        ),
        ev_write(
            event_new(
                base_event,
                clsock,
                EV_WRITE | EV_PERSIST | EV_TIMEOUT,
                Session::cb_write,
                this
            ),
            event_free
        ),
        reader(*this)
    {
        const timeval* timeout = nullptr;
        std::cerr << "Adding" << std::endl;
        if (event_add(ev_read.get(), timeout) == -1) {
            std::cerr << "Cannot add? " << strerror(errno) << std::endl;
        }
    }

    void Session::cb_read(int clsock, short what, void *arg)
    {
        Session& session = *reinterpret_cast<Session*>(arg);
        assert(!(what & (EV_WRITE | EV_SIGNAL)));
        if (what & EV_READ) {
            session.reader.read(clsock);
        }
        else if (what & EV_TIMEOUT) {
        }
    }

    void Session::got_line(const char *line, size_t len)
    {
        std::cerr << "Got line: " << line
            << " len " << len
            << std::endl;
        if (event_add(ev_read.get(), NULL) == -1) {
            std::cerr << "Cannot add again? " << strerror(errno) << std::endl;
        }
    }

    void Session::got_eof()
    {
        std::cerr << "End of file" << std::endl;
    }

    void Session::cb_write(int clsock, short what, void *arg)
    {
        //Session& context = *reinterpret_cast<Session*>(arg);
        std::cerr << "Writing to you..." << std::endl;

        assert(!(what & (EV_READ | EV_SIGNAL)));
        if (what & EV_WRITE) {
            send(clsock, "hi there\n", strlen("hi there\n"), 0);
        }
        else if (what & EV_TIMEOUT) {
        }
    }

} // namespace session
