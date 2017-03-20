#pragma once

#include <string>
#include <list>
#include <ostream>
#include <memory>

#include "node.h"
#include "../settings.h"

namespace goofy::gopher
{
    class NodeMenu : public Node
    {
        public:
            NodeMenu(
                const settings::Settings &settings,
                const std::string& display_name,
                const std::string& selector,
                const std::string& host,
                uint16_t port
            );

            void insert(const Node& node);
            void insert(const NodeInfo& info);

            using WriteParams = gopher::proto::WriteParams;
            using Writer = gopher::proto::Writer;
            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& req
            ) const override;

        private:
            const settings::Settings& settings;
            std::list<NodeInfo> subs;
    };
}
