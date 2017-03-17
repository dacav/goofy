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
                const VirtualPathsMap& vpaths,
                const std::string& path
            );

            virtual ~NodeGopherMap() = default;

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) const override;

        private:
            const settings::Settings& settings;
            const VirtualPathsMap& vpaths;
            const std::string file_path;
    };

}
