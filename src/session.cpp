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
            struct event_base *ev_base) :
        gopher_map(map),
        drop_callback(drop_cb),
        clsock(sock),
        read_params({
            .ev_base=ev_base,
            .timeout={5, 0},
            .got_line=std::bind(&Session::got_line, this,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            .got_eof=std::bind(&Session::got_eof, this),
            .got_timeout=std::bind(&Session::got_timeout, this),
            .got_error=std::bind(&Session::got_error, this,
                std::placeholders::_1
            )
        }),
        reader(read_params, 256u),
        ev_write(
            event_new(
                ev_base,
                clsock,
                EV_WRITE | EV_TIMEOUT,
                Session::cb_write,
                this
            ),
            event_free
        )
    {
        reader.read_from(sock); // FIXME: might throw IOError
    }

    void Session::close()
    {
        ::shutdown(clsock, SHUT_RDWR);
        clsock = -1;
        drop_callback();
    }

    bool Session::got_line(const char *line, size_t len)
    {
        std::cerr << "Got query: " << std::string(line, len) << std::endl;
        try {
            // WORK IN PROGRESS: async send of generic nodes
            //gopher_map.lookup(std::string(line, len)).show(clsock);
            //spg::gopher::proto::writedone(clsock);
        }
        catch (spg::gopher::LookupFailure& e) {
            close();
        }
        return false; // no more read.
    }

    void Session::got_eof()
    {
        std::cerr << "End of file" << std::endl;
        close();
    }

    void Session::got_timeout()
    {
        std::cerr << "Timed out" << std::endl;
        close();
    }

    void Session::got_error(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        close();
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
