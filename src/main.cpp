#include "error.h"

#include "gopher/node-dirlist.h"
#include "gopher/node-term-write.h"
#include "gopher/map.h"

#include "parse/ip.h"
#include "session.h"

#include <iostream>
#include <unordered_map>
#include <list>
#include <memory>
#include <sstream>
#include <cassert>
#include <utility>

#include <event2/event.h>
#include <event2/listener.h>

#include <signal.h>
#include <ext/stdio_filebuf.h>

namespace
{
    struct GlobalContext
    {
        spg::gopher::Map gopher_map;

        using EventBase = std::unique_ptr<
            struct event_base,
            void(*)(struct event_base*)
        >;
        EventBase base_event;

        using SessionMap = std::unordered_map<
            unsigned,
            std::unique_ptr<spg::session::Session>
        >;
        SessionMap sessions;

        GlobalContext()
            : base_event(event_base_new(), event_base_free)
        {}

        void drop_session(unsigned session_id)
        {
            std::cerr << "kaboom " << session_id;
        }

        spg::session::Session& new_session(int clsock)
        {
            using namespace std::placeholders;

            const unsigned next_id = sessions.size();
            std::unique_ptr<spg::session::Session> session(
                new spg::session::Session(
                    gopher_map,
                    std::bind(&GlobalContext::drop_session, this, next_id),
                    clsock,
                    base_event.get()
                )
            );
            auto out = sessions.emplace(next_id, std::move(session));
            assert(out.second);
            return *session.get();
        }
    };

    void cb_accept(
            struct evconnlistener* listener,
            int clsocket,
            struct sockaddr *claddress,
            int claddrlen,
            void *context)
    {
        try {
            reinterpret_cast<GlobalContext*>(context)->new_session(clsocket);
        }
        catch (spg::session::SessionError& error) {
            std::cerr << "Creating session: "
                << error.what() << std::endl;
        }
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

    void cb_signal(int sig, short what, void *ctx)
    {
        GlobalContext& globals = *reinterpret_cast<GlobalContext*>(ctx);

        assert(what & EV_SIGNAL);
        std::cerr << "Got signal " << sig << std::endl;
        event_base_loopbreak(globals.base_event.get());
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

    GlobalContext globals;
    {
        using namespace spg::gopher;
        auto& root = globals.gopher_map.mknode<NodeDirList>("root", "", ip, port);
        auto& l1 = globals.gopher_map.mknode<NodeDirList>("le boobs", "le/boobs", ip, port);
        auto& l2 = globals.gopher_map.mknode<NodeDirList>("le boobies", "le/boobies", ip, port);
        root.insert(l1);
        l1.insert(l2);
        l2.insert(l1);
    }

    struct sockaddr_storage addr_storage;
    std::unique_ptr<struct evconnlistener, void(*)(struct evconnlistener*)> listener(
        evconnlistener_new_bind(
            globals.base_event.get(),
            cb_accept,
            &globals,
            LEV_OPT_CLOSE_ON_FREE | unsigned(sock_reusable) * LEV_OPT_REUSEABLE,
            backlog,
            spg::parse::ipaddr(addr_storage, ip, port),
            sizeof(addr_storage)
        ),
        evconnlistener_free
    );
    evconnlistener_set_error_cb(listener.get(), cb_accept_err);

    std::unique_ptr<struct event, void(*)(struct event*)> sighandler(
        evsignal_new(globals.base_event.get(), SIGINT, cb_signal, &globals),
        event_free
    );
    event_add(sighandler.get(), NULL);

    if (listener.get() == nullptr) {
        std::cerr << "Cannot listen: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // todo: stderr of libevent
    switch (event_base_dispatch(globals.base_event.get())) {
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
