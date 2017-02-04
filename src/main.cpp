#include "error.h"
#include "server.h"

#include "gopher/node-dirlist.h"
#include "gopher/node-term-write.h"
#include "gopher/map.h"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include <event2/event.h>

#include <unistd.h> // tmp, use for write
#include <ext/stdio_filebuf.h>

const uint16_t PORT = 70;

namespace
{
    spg::gopher::Map map;

    void cb_accept(evutil_socket_t fd, short what, void *arg)
    {
        spg::Server &srv = *reinterpret_cast<spg::Server *>(arg);
        spg::Client cl = srv.accept();

        char buffer[1024];
        int n = read(cl.fd, buffer, sizeof buffer);
        std::cerr << "Received query: [" << buffer << "]" << std::endl;
        map.lookup(std::string(buffer, n - 2)).show(cl.fd);
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
    spg::Server srv = spg::Server::parse_addr(argv[1], PORT, 8);
    {
        using namespace spg::gopher;
        auto& root = map.mknode<NodeDirList>("root", "", "::1", PORT);
        auto& l1 = map.mknode<NodeDirList>("le boobs", "le/boobs", "::1", PORT);
        auto& l2 = map.mknode<NodeDirList>("le boobies", "le/boobies", "::1", PORT);
        root.insert(l1);
        l1.insert(l2);
        l2.insert(l1);
    }

    event_base *base = event_base_new();
    main_loop(srv, base);
    event_base_free(base);
}
