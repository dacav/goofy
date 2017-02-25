#include "node-fsys.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "proto.h"
#include "error.h"
#include "map.h"

namespace
{
    mode_t mode_of(const std::string& fsys_path)
    {
        struct stat statbuf;
        /* NOTE: using stat, not lstat. Links are resolved automatically */
        if (stat(fsys_path.c_str(), &statbuf) == -1) {
            throw spg::IOError("stat", errno);
        }
        return statbuf.st_mode;
    }

    std::string requested_path(const spg::gopher::request::Request& request)
    {
        auto cursor = request.query.cbegin();
        cursor ++; // skip selector

        size_t len = 0;
        while (cursor != request.query.cend()) {
            len += 1 + cursor->length();
            cursor ++;
        }
        cursor = request.query.cbegin();
        cursor ++; // skip selector

        std::string out;
        out.reserve(len);
        while (cursor != request.query.cend()) {
            out += '/';
            out += *cursor;
            if (*cursor == "..") {
                throw spg::LookupFailure(out);
            }
            cursor ++;
        }

        return out;
    }

    spg::gopher::NodeType node_type_of(mode_t mode)
    {
        switch (mode & S_IFDIR) {
            case S_IFREG:
                return spg::gopher::NodeType::NT_PLAIN;
            case S_IFDIR:
                return spg::gopher::NodeType::NT_MENU;
            default:
                return spg::gopher::NodeType::NT_ERROR;
        }
    }

    spg::gopher::NodeType node_type_of(const std::string& fsys_path)
    {
        return node_type_of(mode_of(fsys_path));
    }
}

namespace spg::gopher
{
    NodeFSys::NodeFSys(
            const Map& map,
            const std::string& display_name,
            const std::string& selector,
            const std::string& path,
            const std::string& host,
            uint16_t port) :
        Node(
            NodeType::NT_MENU,
            display_name,
            selector,
            host,
            port
        ),
        root_path(path.back() == '/' ? path : path + '/')
    {
        if ((mode_of(path) & S_IFMT) != S_IFDIR) {
            throw IOError(path + ": invalid root node", ENOTDIR);
        }
    }

    NodeFSys::RequestData NodeFSys::analyze_request(const request::Request& request)
    {
        std::string reqpath = requested_path(request);

        std::string fsys_path(root_path);
        fsys_path += reqpath;

        std::string selector_path = info.selector;
        selector_path += '/';
        selector_path += reqpath;

        NodeType type;
        try {
            type = node_type_of(mode_of(fsys_path));
            if (type == NodeType::NT_MENU) {
                fsys_path += '/';
                selector_path += '/';
            }
        }
        catch (IOError& e) {
            switch (e.errno_was) {
                case ENOENT:
                case ENAMETOOLONG:
                case ENOTDIR:
                    throw LookupFailure(selector_path);
                default:
                    throw e;
            }
        }

        RequestData out = {
            .node_type = type,
            .fsys_path = std::move(fsys_path),
            .selector_path = std::move(selector_path),
            .requested_path = std::move(reqpath),
        };
        return out;
    }

    std::unique_ptr<proto::Writer> NodeFSys::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        const RequestData data = analyze_request(request);

        switch (data.node_type) {
            case NodeType::NT_MENU:
                return list_dir(wp, data);
            default:
                throw NodeFailure(data.selector_path, "not supported");
        }
    }

    std::unique_ptr<proto::Writer> NodeFSys::list_dir(
            const WriteParams& wp,
            const RequestData& data)
    {
        DIR* dir = opendir(data.fsys_path.c_str());
        if (dir == nullptr) {
            if (errno == ENOENT) {
                throw LookupFailure(data.selector_path);
            }
            throw IOError("opendir", errno);
        }
        std::unique_ptr<DIR, int(*)(DIR*)> raii_close(dir, closedir);

        auto next_dir = [dir]{
            errno = 0;
            struct dirent* dirent = readdir(dir);
            const char* out = nullptr;
            if (dirent != nullptr) {
                out = dirent->d_name;
            }
            else if (errno != 0) {
                throw IOError("readdir", errno);
            }
            return out;
        };

        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        const char* entry = next_dir();
        while (entry) {
            try {
                writer->node(NodeInfo(
                    node_type_of(data.fsys_path + entry),
                    entry,
                    data.selector_path + entry,
                    info.host,
                    info.port
                ));
            }
            catch (IOError& e) {
                // TODO: log and move along
            }
            entry = next_dir();
        }

        return out;
    }

}
