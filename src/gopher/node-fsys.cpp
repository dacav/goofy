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
            NodeType::NT_DIRLIST,
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
            throw IOError("stat", errno);
        }

        switch (statbuf.st_mode & S_IFMT) {
            case S_IFREG:
                // TODO: check binary vs text, currently pretending it's all
                // text.
                return NodeType::NT_PLAIN;
            case S_IFDIR:
                return NodeType::NT_DIRLIST;
            default:
                // TODO: probably log something here...
                return NodeType::NT_UNSUPPORTED;
        }
    }

    std::unique_ptr<proto::Writer> NodeFSys::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        const std::string path = resolve_path(fsys_path, request);

        // TODO: stat, possibly provide a file.
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
            writer->insert(NodeInfo(
                type_of(path + "/" + entry),
                entry,
                resolve_path(info.selector, request) + "/" + entry,
                info.host,
                info.port
            ));
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
