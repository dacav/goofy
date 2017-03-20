#include "map.h"

#include <utility>

namespace spg::gopher
{
    LookupMap::LookupMap(const settings::Settings& sets) :
        settings(sets),
        url_redirector(settings)
    {
    }

    std::unique_ptr<Node>& LookupMap::insert(Node *item)
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

    const Node& LookupMap::lookup(const request::Request& request) const
    {
        // probably URL: sucks, avoid stringly typed
        if (request.is_url) return url_redirector;

        const std::string selector = request.selector();
        auto ptr = nodes.find(selector);
        if (ptr == nodes.end()) {
            throw LookupFailure(selector);
        }
        return *(ptr->second);
    }

    bool VirtualPathsMap::define(
            const std::string& real_path,
            const char* define_as)
    {
        auto found = paths.find(real_path);
        if (found != paths.end()) return false;

        paths.emplace_hint(
            found,
            std::make_pair(
                real_path,
                define_as == nullptr
                ? std::to_string(paths.size())
                : std::string(define_as)
            )
        );
        return true;
    }

    const std::string& VirtualPathsMap::virtual_path_of(
            const std::string& real_path) const
    {
        auto found = paths.find(real_path);
        assert(found != paths.end());
        return found->second;
    }

    Map::Map(const settings::Settings& settings) :
        lookup_map(settings)
    {
    }

}
