#include "typeguesser.h"
#include "../error.h"

namespace spg::util
{
    mode_t mode_of(const std::string& fsys_path)
    {
        return mode_of(fsys_path.c_str());
    }

    mode_t mode_of(const char* fsys_path)
    {
        struct stat statbuf;
        /* NOTE: using stat, not lstat. Links are resolved automatically */
        if (stat(fsys_path, &statbuf) == -1) {
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

    const char* TypeGuesser::guess(const std::string& path) const
    {
        return magic_file(magic, path.c_str());
    }

}
