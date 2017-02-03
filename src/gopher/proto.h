#pragma once

#include <cstddef>
#include <string>

namespace spg::gopher::proto
{
    void write(int fd, const char* bytes, size_t len);
    void write(int fd, const std::string& str);

    void writeln(int fd, const char* bytes, size_t len);
    void writeln(int fd, const std::string& str);

    void writetb(int fd, const char* bytes, size_t len);
    void writetb(int fd, const std::string& str);
}
