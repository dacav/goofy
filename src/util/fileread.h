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
            ~Reader();

            void feed(const std::string& filename);
            void feed(const char* filename);

            bool eof() const;

            // Returns a string_view over a null-terminated byte sequence.
            StrRef next();

        private:
            std::unique_ptr<FILE, int(*)(FILE*)> input;
            char* line;
            size_t len;
    };

}
