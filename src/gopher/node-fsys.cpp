#include "node-fsys.h"

#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

#include "proto.h"
#include "error.h"

namespace
{
    std::string requested_path(const spg::gopher::request::Request& request)
    {
        auto cursor = request.query.cbegin();
        cursor ++; // skip selector

        size_t len = 0;
        while (cursor != request.query.cend()) {
            len += 1 + cursor->length();
            cursor ++;
        }

        std::string out;
        if (len == 0) {
            return out;
        }

        cursor = request.query.cbegin();
        cursor ++; // skip selector

        out.reserve(len);
        while (cursor != request.query.cend()) {
            const std::string& step = *cursor;

            out += step;
            if (step == ".." || step == ".") {
                throw spg::LookupFailure(out);
            }
            out += '/';
            cursor ++;
        }
        out.pop_back();

        return out;
    }
}

namespace spg::gopher
{
    mode_t mode_of(const std::string& fsys_path)
    {
        struct stat statbuf;
        /* NOTE: using stat, not lstat. Links are resolved automatically */
        if (stat(fsys_path.c_str(), &statbuf) == -1) {
            throw IOError("stat", errno);
        }
        return statbuf.st_mode;
    }

    TypeGuesser::TypeGuesser() :
        magic(
            magic_open(
                MAGIC_MIME
                | MAGIC_SYMLINK
                | MAGIC_NO_CHECK_CDF
                | MAGIC_NO_CHECK_COMPRESS
                | MAGIC_NO_CHECK_ELF
                | MAGIC_NO_CHECK_TAR
                | MAGIC_NO_CHECK_TOKENS
            )
        )
    {
        errno = 0; // not documented, but best effort.
        if (magic == nullptr) {
            throw IOError("magic_open", errno);
        }
        magic_load(magic, nullptr);
    }

    TypeGuesser::~TypeGuesser()
    {
        magic_close(magic);
    }

    NodeType TypeGuesser::type_of(const std::string& path) const
    {
        mode_t mode = mode_of(path);
        switch (mode & S_IFMT) {
            case S_IFREG:
                return type_of_file(path);
            case S_IFDIR:
                return gopher::NodeType::NT_MENU;
            default:
                return gopher::NodeType::NT_ERROR;
        }
    }

    bool TypeGuesser::matches(const char* got, const char* pattern)
    {
        return strstr(got, pattern) == got;
    }

    NodeType TypeGuesser::type_of_file(const std::string& path) const
    {
        const char* magic_guess = magic_file(magic, path.c_str());

        if (matches(magic_guess, "text/")) {
            return gopher::NodeType::NT_PLAIN;
        }
        if (matches(magic_guess, "image/gif")) {
            return gopher::NodeType::NT_GIF;
        }
        if (matches(magic_guess, "image/")) {
            return gopher::NodeType::NT_IMAGE;
        }

        return gopher::NodeType::NT_BINARY;
    }

    NodeFSys::NodeFSys(
            const settings::Settings& sets,
            const gopher::TypeGuesser& tguess,
            const std::string& path,
            const std::string& display_name,
            const std::string& selector) :
        Node(
            NodeType::NT_MENU,
            display_name,
            selector,
            sets.host_name,
            sets.listen_port
        ),
        settings(sets),
        type_guesser(tguess),
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
            const request::Request& request)
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

        proto::MenuWriter* writer = new proto::MenuWriter(wp);
        std::unique_ptr<proto::Writer> out(writer);

        const char* entry = next_entry(dir);
        gopher::TypeGuesser tg;
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
