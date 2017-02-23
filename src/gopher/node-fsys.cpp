#include "node-fsys.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "proto.h"
#include "error.h"
#include "map.h"

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
        fsys_path(path),
        dir(nullptr, closedir)
    {
    }

    std::string NodeFSys::resolve_path(
            const std::string& base,
            const request::Request& request)
    {
        auto cursor = request.query.cbegin();
        cursor ++; // skip selector

        size_t len = base.length();
        while (cursor != request.query.cend()) {
            len += 1 + cursor->length();
            cursor ++;
        }

        std::string out;
        out.reserve(len);
        out += base;
        cursor = request.query.cbegin();
        cursor ++; // skip selector
        while (cursor != request.query.cend()) {
            out += '/';
            out += *cursor;

            if (*cursor == "..") {
                throw LookupFailure(out);
            }
            cursor ++;
        }

        return out;
    }

    NodeType NodeFSys::type_of(const std::string& path)
    {
        struct stat statbuf;
        /* NOTE: using stat, not lstat. Links are resolved automatically */
        if (stat(path.c_str(), &statbuf) == -1) {
            switch (errno) {
                case ENOENT:
                case ENAMETOOLONG:
                case ENOTDIR:
                    throw LookupFailure(path);
                default:
                    throw IOError("stat", errno);
            }
        }

        switch (statbuf.st_mode & S_IFMT) {
            case S_IFREG:
                // TODO: check binary vs text, currently pretending it's all
                // text.
                return NodeType::NT_PLAIN;
            case S_IFDIR:
                return NodeType::NT_MENU;
            default:
                throw BadNodeError(path);
        }
    }

    std::unique_ptr<proto::Writer> NodeFSys::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        std::string path = resolve_path(fsys_path, request);

        switch (auto type = type_of(path)) {
            case NodeType::NT_MENU:
                path += '/';
                return make_dir_writer(wp, request, path);
            default:
                throw InternalError(path + ": No support for " + char(type));
        }
    }

    std::unique_ptr<proto::Writer> NodeFSys::make_dir_writer(
            const WriteParams& wp,
            const request::Request& request,
            const std::string& path)
    {
        dir.reset(opendir(path.c_str()));
        if (dir.get() == nullptr) {
            if (errno == ENOENT) {
                throw LookupFailure(path);
            }
            throw IOError("opendir", errno);
        }

        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        const char* entry = next_dir();
        while (entry) {
            try {
                writer->node(NodeInfo(
                    type_of(path + "/" + entry),
                    entry,
                    resolve_path(info.selector, request) + "/" + entry,
                    info.host,
                    info.port
                ));
            }
            catch (BadNodeError& e) {
                // TODO: log it and move along.
            }
            entry = next_dir();
        }

        dir.reset(nullptr);
        return out;
    }

    const char* NodeFSys::next_dir()
    {
        errno = 0;
        struct dirent* dirent = readdir(dir.get());
        if (dirent != nullptr) {
            return dirent->d_name;
        }
        if (errno == 0) {
            return nullptr;
        }
        throw IOError("readdir", errno);
    }

}
