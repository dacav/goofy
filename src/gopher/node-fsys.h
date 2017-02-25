#pragma once

#include <memory>

#include <sys/types.h>
#include <dirent.h>

#include "map.h"
#include "node-types.h"
#include "node.h"
#include "proto.h"

namespace spg::gopher
{
    class NodeFSys : public Node
    {
        public:
            NodeFSys(
                const Map& map,
                const std::string& display_name,
                const std::string& selector,
                const std::string& root_path,
                const std::string& host,
                uint16_t port
            );

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) override;

        private:
            const std::string root_path;

            struct RequestData
            {
                const NodeType node_type;
                const std::string fsys_path;
                const std::string selector_path;
                const std::string requested_path;
            };

            NodeType type_of(const std::string& requested_sub);
            RequestData analyze_request(const request::Request& request);

            std::unique_ptr<Writer> list_dir(
                const WriteParams& wp,
                const RequestData& paths
            );
            std::unique_ptr<Writer> send_file(
                const WriteParams& wp,
                const RequestData& paths
            );
    };

}
