#include "node-fsys.h"

#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

#include "proto.h"
#include "error.h"

namespace goofy::gopher
{
    NodeFSys::NodeFSys(
            const settings::Settings& sets,
            const gopher::GopherTypeGuesser& tguess,
            const std::string& path,
            const std::string& display_name,
            const std::string& selector) :
        Node(
            NodeType::NT_MENU,
            display_name,
            selector,
            sets.host_name,
            sets.tcp_port
        ),
        settings(sets),
        type_guesser(tguess),
        root_path(path.back() == '/' ? path : path + '/')
    {
        if ((util::mode_of(path) & S_IFMT) != S_IFDIR) {
            throw IOError(path + ": invalid root node", ENOTDIR);
        }
    }

    NodeFSys::RequestData NodeFSys::analyze_request(const request::Request& request) const
    {
        std::string reqpath = request.as_path();

        std::string fsys_path(root_path);
        std::string selector_path(info.selector);

        if (reqpath.length() > 0) {
            fsys_path += reqpath;
            selector_path += '/';
            selector_path += reqpath;
        }

        NodeType type;
        try {
            type = type_guesser.type_of(fsys_path);
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
            const request::Request& request) const
    {
        const RequestData data = analyze_request(request);

        switch (data.node_type) {
            case NodeType::NT_MENU:
                return list_dir(wp, data);
            case NodeType::NT_PLAIN:
            case NodeType::NT_BINARY:
            case NodeType::NT_GIF:
            case NodeType::NT_IMAGE:
                return send_file(wp, data);
            case NodeType::NT_ERROR:
            case NodeType::NT_INFO:
            default:
                throw NodeFailure(data.selector_path, "not supported");
        }
    }

    std::unique_ptr<proto::Writer> NodeFSys::list_dir(
            const WriteParams& wp,
            const RequestData& data) const
    {
        DIR* dir = opendir(data.fsys_path.c_str());
        if (dir == nullptr) {
            if (errno == ENOENT) {
                throw LookupFailure(data.selector_path);
            }
            throw IOError("opendir", errno);
        }
        std::unique_ptr<DIR, int(*)(DIR*)> raii_close(dir, closedir);

        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        const char* entry = next_entry(dir);
        gopher::GopherTypeGuesser tg;
        while (entry) {
            try {
                writer->node(NodeInfo(
                    type_guesser.type_of(data.fsys_path + entry),
                    entry,
                    data.selector_path + entry,
                    info.host,
                    info.port
                ));
            }
            catch (IOError& e) {
                writer->error("...");
                // TODO: log and move along
            }
            entry = next_entry(dir);
        }

        return out;
    }

    bool NodeFSys::hidden(const char* name)
    {
        assert(name != nullptr);
        assert(name[0] != '\0');
        return name[0] == '.';  // includes '.', '..' and dotfiles.
    }

    const char* NodeFSys::next_entry(DIR* dir)
    {
        const char* out = nullptr;

        while (out == nullptr || hidden(out)) {
            errno = 0;
            struct dirent* ent = readdir(dir);
            if (ent != nullptr) {
                out = ent->d_name;
            }
            else if (errno != 0) {
                throw IOError("readdir", errno);
            }
            else {
                return nullptr;
            }
        }

        return out;
    }

    std::unique_ptr<proto::Writer> NodeFSys::send_file(
            const WriteParams& wp,
            const RequestData& data)
    {
        int fd = open(data.fsys_path.c_str(), O_RDONLY);
        if (fd == -1) {
            throw IOError("open", errno);
        }
        return std::unique_ptr<proto::Writer>(new proto::FileWriter(wp, fd));
    }

}
