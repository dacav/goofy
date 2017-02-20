#pragma once

#include "node.h"
#include "map.h"

#include <string>
#include <list>
#include <ostream>
#include <memory>

namespace spg::gopher
{
    class NodeMenu : public Node
    {
        public:
            NodeMenu(const Map& map,
                     const std::string& display_name,
                     const std::string& selector,
                     const std::string& host,
                     uint16_t port);

            void insert(const Node& item);

            using WriteParams = gopher::proto::WriteParams;
            using Writer = gopher::proto::Writer;
            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& req
            ) override;

        private:
            const Map& map;
            std::list<std::string> subs;
    };
}
