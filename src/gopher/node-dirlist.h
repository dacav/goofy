#pragma once

#include "node.h"

#include <string>
#include <list>
#include <ostream>
#include <memory>

namespace spg::gopher
{
    class NodeDirList : public Node
    {
        public:
            NodeDirList(const std::string& display_name,
                        const std::string& selector,
                        const std::string& host,
                        uint16_t port);

            const std::list<std::shared_ptr<Node>>& list_nodes() const;
            void insert(const std::shared_ptr<Node>& item);

            virtual void show(int fd) const override;

        private:
            std::list<std::shared_ptr<Node>> nodes;

    };
}
