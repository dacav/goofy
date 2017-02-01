#pragma once

#include <map>
#include <memory>

#include <cstddef>

#include "node.h"
#include "error.h"

namespace spg::gopher
{
    class MapError : public spg::Error {
        public:
            MapError(const std::string msg) : spg::Error(msg) {}
    };

    class Map
    {
        public:
            Map();

            using NodeLink = std::shared_ptr<spg::gopher::Node>;

            const NodeLink& insert(Node* item);
            NodeLink lookup(const std::string& sel);

            const size_t max_selector_length() const {
                return maxlen;
            }

        private:
            std::map<std::string, NodeLink> nodes;
            size_t maxlen;
    };
}
