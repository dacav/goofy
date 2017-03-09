#pragma once

#include <string>
#include <functional>

#include "map.h"
#include "node-types.h"
#include "node.h"
#include "proto.h"

namespace spg::gopher
{
    class NodeGopherMap : public Node
    {
        public:
            NodeGopherMap(
                const Map& map,
                std::string&& selector,
                NodeInfo&& info
            );

            virtual ~NodeGopherMap() = default;

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) override;

        private:
            const Map& map;
            const std::string file_path;
    };

}
