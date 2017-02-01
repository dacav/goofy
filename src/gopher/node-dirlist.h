#pragma once

#include "node.h"

#include <string>
#include <list>
#include <ostream>
#include <memory>

namespace spg::gopher
{
    class NodeDirList : public Node {
        public:
            NodeDirList(const std::string& display_name,
                        const std::string& selector,
                        const std::string& host,
                        uint16_t port);

            using NodeLink = std::shared_ptr<spg::gopher::Node>;
            using NodeList = std::list<NodeLink>;

            const NodeList list_nodes() const;
            void insert(const NodeLink& item);

        private:
            NodeList nodes;

    };
}
