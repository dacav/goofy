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

    const Map::NodeLink& Map::insert(Node* item)
    {
        const std::string& selector = item->selector;

        auto position = nodes.find(selector);
        if (position != nodes.end()) {
            std::ostringstream err;
            err << "Duplicated selector for "
                << *item << " and " << *(position->second);
            throw MapError(err.str());
        }

        std::shared_ptr<Node>& ptr = nodes[selector];
        ptr.reset(item);
        maxlen = std::max(selector.length(), maxlen);

        return ptr;
    }

    Map::NodeLink Map::lookup(const std::string& selector)
    {
        auto position = nodes.find(selector);
        if (position == nodes.end()) {
            return nullptr;
        }
        return position->second;
    }
}
