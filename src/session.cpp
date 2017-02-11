#include "session.h"

#include <cerrno>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <sys/socket.h>

#include <iostream>

namespace spg::session
{

    Session::Session(
            spg::gopher::Map& map,
            const DropCallback& drop_cb,
            int sock,
            struct event_base *base_event) :
        gopher_map(map),
        drop_callback(drop_cb),
        clsock(sock),
        ev_read(
            event_new(
                base_event,
                clsock,
                EV_READ | EV_PERSIST | EV_TIMEOUT,
                Session::cb_read,
                this
            ),
            event_free
        ),
        ev_write(
            event_new(
                base_event,
                clsock,
                EV_WRITE | EV_TIMEOUT,
                Session::cb_write,
                this
            ),
            event_free
        ),
        reader(*this)
    {
        if (clsock < 0) {
            throw SessionError("Invalid client socket");
        }

        const timeval timeout = {5, 1}; // here by conf.
        if (event_add(ev_read.get(), &timeout) == -1) {
            throw SessionError("Adding event", errno);
        }
    }

    void Session::close()
    {
        event_del(ev_read.get());
        ::shutdown(clsock, SHUT_RDWR);
        clsock = -1;
        drop_callback();
    }

    void Session::cb_read(int clsock, short what, void *arg)
    {
        Session& session = *reinterpret_cast<Session*>(arg);
        assert(!(what & (EV_WRITE | EV_SIGNAL)));
        if (what & EV_READ) {
            session.reader.read(clsock);
        }
        else if (what & EV_TIMEOUT) {
            std::cerr << "Timed out!" << std::endl;
            session.close();
        }
    }

    void Session::got_line(const char *line, size_t len)
    {
        event_del(ev_read.get());
        std::cerr << "Got query: " << std::string(line, len) << std::endl;
        try {
            gopher_map.lookup(std::string(line, len)).show(clsock);
            spg::gopher::proto::writedone(clsock);
        }
        catch (spg::gopher::LookupFailure& e) {
            close();
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
        }
        else if (what & EV_TIMEOUT) {
        }
    }

} // namespace session
