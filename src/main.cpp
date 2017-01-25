#include "error.h"
#include "server.h"

#include <iostream>
#include <event2/event.h>

namespace
{
    void cb_accept(evutil_socket_t fd, short what, void *arg)
    {
        spg::Server &srv = *reinterpret_cast<spg::Server *>(arg);

        std::cerr << "Got connect on " << &srv << std::endl;
    }

    void main_loop (spg::Server &srv, event_base *base)
    {
        struct event *ev_accept = event_new(base, srv.fd,
            EV_READ | EV_PERSIST,
            cb_accept,
            reinterpret_cast<void *>(&srv)
        );

        event_add(ev_accept, NULL);

        switch (event_base_dispatch(base)) {
            case -1: // error
                std::cerr << "error" << std::endl;
                break;
            case 0: // exited normally
                std::cerr << "exited normlly" << std::endl;
                break;
            case 1: // no more events
                std::cerr << "no more events" << std::endl;
                break;
        }
    }
}

int main(int argc, char **argv)
{
    spg::Server srv = spg::Server::parse_addr(argv[1], 8070, 8);

    std::cerr << "go go gadget " << &srv << std::endl;

    event_base *base = event_base_new();
    main_loop(srv, base);
    event_base_free(base);
}
