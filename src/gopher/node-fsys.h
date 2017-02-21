#pragma once

#include <memory>

#include <sys/types.h>
#include <dirent.h>

#include "proto.h"
#include "node.h"
#include "map.h"

namespace spg::gopher
{

    class NodeFSys : public Node
    {
        public:
            NodeFSys(
                const Map& map,
                const std::string& display_name,
                const std::string& selector,
                const std::string& fsys_path,
                const std::string& host,
                uint16_t port
            );

            virtual std::unique_ptr<Writer> make_writer(
                const WriteParams& wp,
                const request::Request& request
            ) override;

        private:
            const std::string fsys_path;
            std::unique_ptr<DIR, int(*)(DIR *)> dir;

            static std::string resolve_path(
                const std::string& base,
                const request::Request& request
            );
            const char* next_dir();
    };

}
