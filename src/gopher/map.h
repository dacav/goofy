#pragma once

#include <unordered_map>
#include <memory>
#include <utility>

#include <cstddef>

#include "node.h"
#include "node-redirect.h"
#include "request.h"
#include "../error.h"
#include "../settings.h"

namespace goofy::gopher
{
    class MapError : public goofy::Error {
        public:
            MapError(const std::string msg)
                : goofy::Error(msg) {}
    };

    class DuplicatedError : public MapError {
        public:
            DuplicatedError(const std::string& selector)
                : MapError(std::string("Duplicated selector: ") + selector) {}
    };

    class LookupMap
    {
        public:
            LookupMap(const settings::Settings& settings);

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
            const Node& lookup(const request::Request& request) const;

        private:
            const settings::Settings& settings;
            const NodeRedirect url_redirector;

            // maps selectors to nodes, used for lookups, populated by
            // mknode.
            std::unordered_map<std::string, std::unique_ptr<Node>> nodes;
            std::unique_ptr<Node>& insert(Node *item);
    };

    class VirtualPathsMap
    {
        public:
            VirtualPathsMap() = default;

            bool define(
                const std::string& real_path,
                const char* define_as=nullptr
            );

            const std::string& virtual_path_of(const std::string& real_path) const;

        private:
            std::unordered_map<std::string, std::string> paths;
    };

    class Map
    {
        public:
            Map(const settings::Settings& settings);
            Map(const Map&) = delete;
            Map(Map&&) = delete;
            void operator=(const Map&) = delete;
            void operator=(Map&&) = delete;

            LookupMap lookup_map;
            VirtualPathsMap paths_map;
    };

}
