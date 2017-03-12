#pragma once

#include <map>
#include <memory>
#include <utility>

#include <cstddef>

#include "node.h"
#include "../error.h"
#include "../settings.h"

namespace spg::gopher
{
    class MapError : public spg::Error {
        public:
            MapError(const std::string msg)
                : spg::Error(msg) {}
    };

    class DuplicatedError : public MapError {
        public:
            DuplicatedError(const std::string& selector)
                : MapError(std::string("Duplicated selector: ") + selector) {}
    };

    class Map
    {
        public:
            Map(const settings::Settings& settings);
            Map(const Map&) = delete;
            Map(Map&&) = delete;
            void operator=(const Map&) = delete;
            void operator=(Map&&) = delete;

            // Create new node and bind its selector for user lookup
            template <typename NodeT, typename... Args>
            NodeT& mknode(Args&&... args)
            {
                NodeT *node = new NodeT(settings, std::forward<Args>(args)...);
                return dynamic_cast<NodeT &>(
                    *insert(dynamic_cast<Node*>(node))
                );
            }

            // User lookup, may throw LookupFailure
            Node& lookup(const std::string& selector) const;

        private:
            const settings::Settings& settings;

            // maps selectors to nodes, used for lookups, populated by
            // mknode.
            std::map<std::string, std::unique_ptr<Node>> nodes;
            std::unique_ptr<Node>& insert(Node *item);
    };
}
