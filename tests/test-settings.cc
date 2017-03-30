#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <cassert>
#include <array>

#include "error.h"
#include "settings.h"
#include "util/str.h"

using namespace goofy;

class TmpFile {
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

template <typename T>
void test_serialization(const char* name, const T& value)
{
    TmpFile file;

    const size_t len = settings::ConfItem<T>(name, value).store_to(file);

    std::unique_ptr<char[]> large_enough(new char[len]);
    if (std::fseek(file, 0L, SEEK_SET) == -1) {
        throw IOError("fseek", errno);
    }
    if (std::fread(large_enough.get(), len, 1, file) != 1) {
        throw IOError("fread");
    }

    util::StrRef reloaded(large_enough.get(), len);

    // Skipping the label
    reloaded += strlen(name);
    reloaded.ltrim();

    settings::ConfItem<T> item(name, T());

    // If this is not the case, our test doesn't mean anything:
    // It would not prove that we could parse it correctly.
    assert(item.value != value);

    std::cerr << "dumped: ["
        << std::string(reloaded)
        << "]" << std::endl;
    item.parse_assign(reloaded.start, reloaded.len);
    std::cerr
        << "original value: " << value << std::endl
        << "reloaded value: " << item.value << std::endl;
    assert(item.value == value);
}

int main(int argc, char** argv)
{
    test_serialization<uint16_t>("net.tcp_port", 7070);
    test_serialization<unsigned>("foo.bar", 1024);
}