#include "request.h"
#include "../error.h"

#include <cassert>

namespace spg::gopher::request
{
    Request::Request(const char* line, size_t len) :
        raw_body(line, len),
        is_url(raw_body.find("URL:") == 0),
        separator(is_url ? ':' : '/'),
        first(
            is_url
            ? strlen("URL:")
            : raw_body.find_first_not_of(separator)
        )
    {
        assert(raw_body.length() > 0 || first == raw_body.npos);
    }

    std::string Request::selector() const
    {
        if (first == raw_body.npos) return "";
        if (is_url) return "URL:";
        return raw_body.substr(
            first,
            raw_body.find_first_of(separator, first) - first
        );
    }

    std::string Request::url() const
    {
        if (!is_url) return "";
        return raw_body.substr(first);
    }

    std::list<util::StrRef> Request::query() const
    {
        if (is_url) return {};
        auto tokens = util::tokenize(raw_body, separator);
        tokens.pop_front();
        return tokens;
    }

    std::string Request::as_path() const
    {
        if (is_url) return "";

        std::string out;
        out.reserve(raw_body.length());
        for (std::string tok : query()) {
            if (tok.length() == 0) continue;
            if (tok == ".." || tok == ".") {
                // note, '/' is the separator.
                throw BadRequest(raw_body, "invalid path");
            }
            out += tok;
            out += '/';
        }
        out.pop_back(); // drop trailing slash.

        return out;
    }
}
