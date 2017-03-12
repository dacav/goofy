#pragma once

#include <string>
#include <functional>

#include "node-types.h"
#include "node.h"
#include "proto.h"
#include "../settings.h"

namespace spg::gopher
{
    class NodeGopherMap : public Node
    {
        public:
            NodeGopherMap(
                const settings::Settings& settings,
                std::string&& file_path,
                NodeInfo&& info
            );

            virtual ~NodeGopherMap() = default;

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) override;

        private:
            const settings::Settings& settings;
            const std::string file_path;
    };

}
