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
            MapError(const std::string msg)
                : spg::Error(msg) {}
    };

    class Duplicated : public MapError {
        Duplicated(const std::string& selector)
            : MapError(std::string("Duplicated selector: ") + selector) {}
    };

    class LookupFailure : public spg::UserError {
        public:
            LookupFailure(const std::string& selector)
                : spg::UserError("Lookup Failure", selector) {}
    };

    class Map
    {
        public:
            Map();

            Map(const Map&) = delete;
            Map(Map&&) = delete;
            void operator=(const Map&) = delete;
            void operator=(Map&&) = delete;

            template <typename NodeT, typename... Args>
            NodeT& mknode(Args&&... args)
            {
                NodeT *node = new NodeT(*this, args...);
                return dynamic_cast<NodeT &>(
                    *insert(dynamic_cast<Node*>(node))
                );
            }

            const size_t max_selector_length() const;

            Node& lookup(const std::string& selector) const;

        private:
            std::map<std::string, std::unique_ptr<Node>> nodes;
            size_t maxlen;
            std::unique_ptr<Node>& insert(Node *item);
    };
}
