#pragma once

#include <cstddef>
#include <string>
#include <memory>

#include "proto.h"
#include "node-types.h"
#include "request.h"

#include "../error.h"

namespace spg::gopher
{

    class NodeError : public spg::Error {
        public:
            NodeError(const std::string msg) : spg::Error(msg) {}
    };

    class Node {
        public:
            Node(NodeType type,
                 const std::string& display_name,
                 const std::string& selector,
                 const std::string& host,
                 uint16_t port);
            virtual ~Node();

            const char type;
            const std::string display_name;
            const std::string selector;
            const std::string host;
            const uint16_t port;

            const std::string repr;

            using WriteParams = spg::gopher::proto::WriteParams;
            using Writer = spg::gopher::proto::Writer;
            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) = 0;
    };

}
