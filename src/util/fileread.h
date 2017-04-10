#pragma once

#include <memory>
#include <utility>
#include <string>

#include "str.h"

namespace goofy::util
{

    // Simple RAII wrapper around cstdio's file descriptors.
    //
    // ...and yeah, I believe iostream is just useless complexity here.
    class Reader
    {
        public:
            Reader();
            Reader(const std::string& filename);
            ~Reader();

            void feed(const std::string& filename);
            void feed(const char* filename);

            bool eof() const;

            // Returns a StrRef over the next line. The underlying string is
            // always null-terminated and does not have a trailing new-line
            // symbol. The reference is valid up to the subsequent call of
            // next().
            StrRef next();

        private:
            std::unique_ptr<FILE, int(*)(FILE*)> input;
            char* line;
            size_t len;
    };

}
