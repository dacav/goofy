#include "error.h"

#include "gopher/node-menu.h"
#include "gopher/node-fsys.h"
#include "gopher/map.h"

#include "settings.h"
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
    struct Server
    {
        Server(const spg::settings::Settings& settings);

        spg::gopher::Map gopher_map;

        std::unique_ptr<
            struct event_base,
            void(*)(struct event_base*)
        > base_event;

        std::unique_ptr<
            struct evconnlistener,
            void(*)(struct evconnlistener*)
        > tcp_listener;

        std::unique_ptr<
            struct event,
            void(*)(struct event*)
        > sighandler;

        std::unordered_map<
            unsigned,
            std::unique_ptr<spg::session::Session>
        > sessions;

        spg::gopher::TypeGuesser type_guesser;

        void drop_session(unsigned session_id);
        spg::session::Session& new_session(int clsock);
        void start();

        static void cb_accept(
                struct evconnlistener* tcp_listener,
                int clsocket,
                struct sockaddr *claddress,
                int claddrlen,
                void *context);
        static void cb_accept_err(
                struct evconnlistener *tcp_listener,
                void *context);
        static void cb_signal(
                int sig,
                short what,
                void *ctx);
    };

    Server::Server(const spg::settings::Settings& settings) :
        base_event(event_base_new(), event_base_free),
        tcp_listener(nullptr, evconnlistener_free),
        sighandler(nullptr, event_free)
    {
        tcp_listener.reset(evconnlistener_new_bind(
            base_event.get(),
            Server::cb_accept,
            this,
            LEV_OPT_CLOSE_ON_FREE
                | unsigned(settings.sock_reusable) * LEV_OPT_REUSEABLE,
            settings.accept_backlog,
            reinterpret_cast<const sockaddr *>(&settings.bind_addr),
            sizeof(settings.bind_addr)
        ));
        if (tcp_listener.get() == nullptr) {
            std::cerr << "Cannot listen: " << strerror(errno) << std::endl;
            throw 1;
        }
        evconnlistener_set_error_cb(tcp_listener.get(), cb_accept_err);

        sighandler.reset(evsignal_new(
            base_event.get(), SIGINT, cb_signal, this
        ));
        signal(SIGPIPE, SIG_IGN);
        event_add(sighandler.get(), NULL);
    }

    void Server::drop_session(unsigned session_id)
    {
        sessions.erase(session_id);
    }

    spg::session::Session& Server::new_session(int clsock)
    {
        using namespace std::placeholders;

        assert(clsock != -1);
        std::unique_ptr<spg::session::Session> session(
            new spg::session::Session(
                gopher_map,
                std::bind(&Server::drop_session, this, clsock),
                clsock,
                base_event.get()
            )
        );
        auto out = sessions.emplace(clsock, std::move(session));
        assert(out.second);
        return *session.get();
    }

    void Server::cb_accept(
            struct evconnlistener* tcp_listener,
            int clsocket,
            struct sockaddr *claddress,
            int claddrlen,
            void *context)
    {
        try {
            reinterpret_cast<Server*>(context)->new_session(clsocket);
        }
        catch (spg::session::SessionError& error) {
            std::cerr << "Creating session: "
                << error.what() << std::endl;
        }
    }

    void Server::cb_accept_err(
            struct evconnlistener *tcp_listener,
            void *context)
    {
        struct event_base *base = evconnlistener_get_base(tcp_listener);
        std::cerr << "Error while accepting: "
            << strerror(errno) << std::endl;
        event_base_loopexit(base, NULL);
    }

    void Server::cb_signal(
            int sig,
            short what,
            void *ctx)
    {
        Server& srv = *reinterpret_cast<Server*>(ctx);

        assert(what & EV_SIGNAL);
        std::cerr << "Got signal " << sig << std::endl;
        event_base_loopbreak(srv.base_event.get());
    }

    void Server::start()
    {
        // todo: take stderr of libevent for logging.
        switch (event_base_dispatch(base_event.get()))
        {
            case -1: // error
                std::cerr << "error from libevent" << std::endl;
                throw 1;
            case 0: // exited normally
                std::cerr << "terminated normally" << std::endl;
                throw 0;
            case 1: // no more events
                std::cerr << "no more events" << std::endl;
                throw 0;
        }
    }

} // anon namespace

int main(int argc, char **argv)
{
    spg::settings::Settings settings;
    settings.listen_port = 7070;
    settings.bind_addr = spg::settings::mkaddr("::1", settings.listen_port);
    settings.accept_backlog = 10;
    settings.sock_reusable = true;

    Server srv(settings);

    {
        using namespace spg::gopher;
        auto& root = srv.gopher_map.mknode<NodeMenu>("root", "", "localhost", settings.listen_port);
        auto& l1 = srv.gopher_map.mknode<NodeMenu>("le boobs", "le_boobs", "localhost", settings.listen_port);
        auto& l2 = srv.gopher_map.mknode<NodeFSys>(srv.type_guesser,
                "le boobies", "le_boobies", ".", "localhost", settings.listen_port);
        root.insert(l1);
        l1.insert(l2);
    }

    try {
        srv.start();
    }
    catch (int ret) {
        return ret;
    }
}
