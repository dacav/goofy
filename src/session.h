#pragma once

#include <event2/event.h>
#include <memory>
#include <vector>
#include <cstddef>
#include <functional>
#include <exception>

#include "gopher/proto.h"
#include "gopher/map.h"
#include "error.h"

namespace spg::session
{
    class SessionError : public spg::Error
    {
        public:
            SessionError(const std::string &msg)
                : spg::Error(msg)
            {}
            SessionError(const std::string &when, int e)
                : spg::Error(when, e)
            {}
    };

    class Session
    {
        public:
            using DropCallback = std::function<void()>;

            Session(
                spg::gopher::Map& gopher_map,
                const DropCallback& drop_callback,
                int clsock,
                struct event_base*
            );

            Session(Session&&) = delete;
            Session(const Session&) = delete;
            void operator=(const Session&) = delete;
            void operator=(Session&&) = delete;

        private:
            gopher::Map& gopher_map;
            DropCallback drop_callback;

            int clsock;

            bool got_line(const char* line, size_t len);
            void got_eof();
            void got_timeout();
            void got_error(std::exception& e);

            static void show_error(
                const spg::UserError& e,
                spg::gopher::proto::MenuWriter& menu
            );

            gopher::proto::ReadParams read_params;
            gopher::proto::Reader reader;
            std::unique_ptr<gopher::proto::Writer> writer;

            void close();

    };

} // namespace session
