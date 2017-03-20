#pragma once

#include <string>

#include "proto.h"
#include "node-types.h"
#include "request.h"

#include "../error.h"

namespace goofy::gopher
{

    class NodeError : public goofy::Error {
        public:
            NodeError(const std::string msg) : goofy::Error(msg) {}
    };

    class Node {
        public:
            Node(NodeType type,
                 const std::string& display_name,
                 const std::string& selector,
                 const std::string& host,
                 uint16_t port);
            virtual ~Node();

            const NodeInfo info;

            using WriteParams = goofy::gopher::proto::WriteParams;
            using Writer = goofy::gopher::proto::Writer;
            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) const = 0;
    };

}
