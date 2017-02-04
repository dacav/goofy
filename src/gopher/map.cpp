#include "map.h"

#include "node-term-write.h"

#include <algorithm>
#include <utility>
#include <sstream>

namespace spg::gopher
{
    Map::Map()
        : maxlen(0)
    {
    }

    const size_t Map::max_selector_length() const
    {
        return maxlen;
    }

    std::unique_ptr<Node>& Map::insert(Node *item)
    {
        const std::string &selector = item->selector;
        decltype(nodes)::iterator ins;
        bool success;

        std::tie(ins, success) = nodes.emplace(selector, item);

        if (success) {
            maxlen = std::max(selector.length(), maxlen);
            return ins->second;
        }
        else {
            MapError e(selector);
            delete item;
            throw e;
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
