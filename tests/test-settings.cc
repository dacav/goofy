#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <cassert>
#include <array>
#include <ostream>

#include "error.h"
#include "settings.h"
#include "util/str.h"

using namespace goofy;

class TmpFile
{
    public:
        TmpFile();
        ~TmpFile();

        std::string name;
        operator FILE*() {
            return raii.get();
        }

    private:
        std::unique_ptr<FILE, int(*)(FILE*)> raii;
};

TmpFile::TmpFile()
    : raii(nullptr, fclose)
{
    char buf[] = "goofy_unittest_XXXXXX";
    int fd = mkstemp(buf);
    if (fd == -1) {
        throw IOError("mkstemp", errno);
    }

    raii.reset(fdopen(fd, "w+"));
    if (raii.get() == nullptr) {
        throw IOError("fdopen", errno);
    }
    name = buf;
}

TmpFile::~TmpFile()
{
    if (::unlink(name.c_str()) == -1) {
        std::cerr << "Could not unlink! "
            << std::strerror(errno) << std::endl;
    }
}

bool operator==(const sockaddr_storage& s1, const sockaddr_storage& s2)
{
    return memcmp(&s1, &s2, sizeof(sockaddr_storage)) == 0;
}

bool operator!=(const sockaddr_storage& s1, const sockaddr_storage& s2)
{
    return !(s1 == s2);
}

std::ostream& operator<<(std::ostream& stream, const sockaddr_storage& s)
{
    const char* cursor = (const char*)&s;
    const char* const end = cursor + sizeof(sockaddr_storage);
    while (cursor < end) {
        std::cerr << " 0x" << std::hex << (unsigned(*cursor) & 0xff);
        cursor ++;
    }
    return stream;
}

int main(int argc, char** argv)
{
    TmpFile file;
    settings::Settings sets;

    sets.save(file.name);

    if (std::fseek(file, 0L, SEEK_SET) == -1) {
        throw IOError("fseek", errno);
    }
    const size_t len = 4096;
    std::unique_ptr<char[]> large_enough(new char[len]);
    const size_t got = std::fread(large_enough.get(), 1, len, file);
    if (got == 0) {
        throw IOError("fread");
    }
    if (std::fwrite(large_enough.get(), 1, got, stderr) == 0) {
        throw IOError("write");
    }
}
