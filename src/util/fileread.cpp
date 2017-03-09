#include "fileread.h"
#include "../error.h"

namespace spg::util
{

    Reader::~Reader()
    {
        std::free(line);
    }

    Reader::Reader() :
        input(nullptr, fclose),
        line(nullptr),
        len(0)
    {
    }

    void Reader::feed(const std::string& filename)
    {
        feed(filename.c_str());
    }

    void Reader::feed(const char* filename)
    {
        input.reset(fopen(filename, "r"));
        if (input.get() == nullptr) {
            throw IOError("fopen", errno);
        }
    }

    StrRef Reader::next()
    {
        errno = 0;
        ssize_t read = getline(&line, &len, input.get());
        if (read < 1) {
            if (read < 0 && errno != 0) {
                throw IOError("getline", errno);
            }
            return StrRef(nullptr, 0);
        }

        line[-- read] = 0; // chop
        return StrRef(line, read);
    }

    bool Reader::eof() const
    {
        FILE *f = input.get();
        return f == nullptr ? true : feof(f);
    }

}