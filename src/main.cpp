#include "error.h"
#include "server.h"

#include "gopher/node-dirlist.h"
#include "gopher/node-term-write.h"
#include "gopher/map.h"

#include "parse/ip.h"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include <event2/event.h>
#include <event2/listener.h>

#include <unistd.h> // tmp, use for write
#include <ext/stdio_filebuf.h>

struct GlobalContext
{
    spg::gopher::Map map;
};

namespace
{

    void cb_accept(
            struct evconnlistener* listener,
            int clsocket,
            struct sockaddr *claddress,
            int claddrlen,
            void *context)
    {
        std::cerr << "Connected yo " << clsocket << std::endl;
    }

    void cb_accept_err(
            struct evconnlistener *listener,
            void *context)
    {
        struct event_base *base = evconnlistener_get_base(listener);
        std::cerr << "Error while accepting: "
            << strerror(errno) << std::endl;
        event_base_loopexit(base, NULL);
    }

} // anon namespace

int main(int argc, char **argv)
{
    // Stuff to put in configuration
    const char* ip = "::1";
    const uint16_t port = 8070;
    const unsigned backlog = 10;
    const bool sock_reusable = true;

    if (argc > 1) {
        ip = argv[0];
    }

    GlobalContext context;
    {
        using namespace spg::gopher;
        auto& root = context.map.mknode<NodeDirList>("root", "", ip, port);
        auto& l1 = context.map.mknode<NodeDirList>("le boobs", "le/boobs", ip, port);
        auto& l2 = context.map.mknode<NodeDirList>("le boobies", "le/boobies", ip, port);
        root.insert(l1);
        l1.insert(l2);
        l2.insert(l1);
    }

    struct sockaddr_storage addr_storage;
    std::unique_ptr<struct event_base, void(*)(struct event_base*)> base(
        event_base_new(),
        event_base_free
    );
    std::unique_ptr<struct evconnlistener, void(*)(struct evconnlistener*)> listener(
        evconnlistener_new_bind(
            base.get(),
            cb_accept,
            &context,
            LEV_OPT_CLOSE_ON_FREE | unsigned(sock_reusable) * LEV_OPT_REUSEABLE,
            backlog,
            spg::parse::ipaddr(addr_storage, ip, port),
            sizeof(addr_storage)
        ),
        evconnlistener_free
    );
    evconnlistener_set_error_cb(listener.get(), cb_accept_err);

    if (listener.get() == nullptr) {
        std::cerr << "Cannot listen: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // todo: stderr of libevent
    switch (event_base_dispatch(base.get())) {
        case -1: // error
            std::cerr << "error from libevent" << std::endl;
            exit(EXIT_FAILURE);
            break;
        case 0: // exited normally
            std::cerr << "terminated normally" << std::endl;
            exit(EXIT_SUCCESS);
            break;
        case 1: // no more events
            std::cerr << "no more events" << std::endl;
            exit(EXIT_SUCCESS);
            break;
    }
}
