#include "node-fsys.h"

#include <cerrno>
#include <cstring>

#include "proto.h"

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

    std::unique_ptr<proto::Writer> NodeFSys::make_writer(
            const WriteParams& wp,
            const request::Request& request)
    {
        dir.reset(opendir(fsys_path.c_str()));
        if (dir.get() == nullptr) {
            throw IOError("opendir", errno);
        }

        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        const char* entry = next_dir();
        while (entry) {
            writer->insert(NodeInfo(
                NT_DIRLIST,
                entry,
                info.selector + "/" + entry,
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
