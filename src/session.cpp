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
        reader(read_params, 256u)
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
        try {
            spg::gopher::proto::WriteParams params = {
                .ev_base = read_params.ev_base,
                .timeout = read_params.timeout,
                .got_success = std::bind(&Session::close, this),
                .got_error = read_params.got_error,
                .got_timeout = read_params.got_timeout
            };
            writer = std::move(
                gopher_map.lookup(std::string(line, len)).writer(params)
            );
            writer->write_to(clsock);
        }
        catch (spg::gopher::LookupFailure& e) {
            close();
        }
        // XXX more catch?
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

} // namespace session
