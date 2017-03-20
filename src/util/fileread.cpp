#include "fileread.h"
#include "../error.h"

namespace goofy::util
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
        FILE* f = input.get();
        if (f == nullptr) {
            return StrRef();
        }
        errno = 0;
        ssize_t read = getline(&line, &len, input.get());
        if (read < 1) {
            if (read < 0 && errno != 0) {
                throw IOError("getline", errno);
            }
            input.reset(nullptr);
            return StrRef();
        }

        line[-- read] = 0; // chop
        return StrRef(line, read);
    }

    bool Reader::eof() const
    {
        FILE* f = input.get();
        return f == nullptr ? true : feof(f);
    }

}
