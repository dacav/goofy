#pragma once

#include <string>
#include <memory>

#include "node-types.h"
#include "node.h"
#include "proto.h"

#include "../settings.h"
#include "../map_parser.h"

namespace spg::gopher
{
    class NodeGopherMap : public Node
    {
        public:
            NodeGopherMap(
                const settings::Settings& sets,
                const std::shared_ptr<map_parser::VirtualPathsMap>& vpaths,
                const std::string& path
            );

            virtual ~NodeGopherMap() = default;

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) override;

        private:
            const settings::Settings& settings;
            const std::shared_ptr<map_parser::VirtualPathsMap> vpaths;
            const std::string file_path;
            const map_parser::Parser map_parser;
            std::unique_ptr<proto::MenuWriter> writer;

            void got_text(std::string&&);
            void got_nodeinfo(gopher::NodeInfo&&, bool);
    };

}
