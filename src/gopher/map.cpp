#include "map.h"

#include <utility>

namespace spg::gopher
{
    std::unique_ptr<Node>& Map::insert(Node *item)
    {
        const std::string &selector = item->info.selector;
        std::unique_ptr<Node> item_ptr(item);

        if (nodes.find(selector) == nodes.end()) {
            auto& new_ptr = nodes[selector];
            new_ptr = std::move(item_ptr);
            return new_ptr;
        }
        else {
            throw DuplicatedError(selector);
        }
    }

    Node& Map::lookup(const std::string& selector) const
    {
        auto ptr = nodes.find(selector);
        if (ptr == nodes.end()) {
            throw LookupFailure(selector);
        }
        return *(ptr->second);
    }
}
