#pragma once

#include "node.h"
#include "../settings.h"

namespace spg::gopher
{
    class NodeRedirect : public Node
    {
        public:
            NodeRedirect(
                const settings::Settings& settings
            );

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& req
            ) override;

        private:
            const settings::Settings& settings;
    };
}
