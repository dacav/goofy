#pragma once

#include <magic.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>

namespace goofy::util
{
    mode_t mode_of(const std::string& fsys_path);
    mode_t mode_of(const char* fsys_path);

    class TypeGuesser
    {
        public:
            TypeGuesser();
            virtual ~TypeGuesser();

            const char* guess(const std::string& path) const;

        private:
            magic_t magic;
    };

}
