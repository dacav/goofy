#include <vector>
#include <cassert>
#include "util/str.h"

#include <iostream>

using namespace goofy;

namespace
{
    void test_base();

    void test_tokenizer(
        const std::string testcase,
        std::vector<std::string> expected
    );
}

int main(int argc, char** argv)
{
    test_base();

    test_tokenizer("hello world", {"hello", "world"});
    test_tokenizer("hello", {"hello"});
    test_tokenizer("   hello world   ",
        {"", "", "", "hello", "world", "", "", ""}
    );

    return 0;
}

namespace
{
    void test_base()
    {
        const std::string hello("   hello world ");

        util::StrRef ref(hello);
        assert(std::string(ref) == hello);

        ref.ltrim();
        assert(std::string(ref) == "hello world ");
    }

    void test_tokenizer(
            const std::string testcase,
            std::vector<std::string> expected)
    {
        std::cerr << "--- test_tokenizer [" << testcase << "] ---" << std::endl;
        unsigned i = 0;
        for (std::string tok : util::tokenize(testcase, ' ')) {
            std::cerr
                << "tok=" << tok
                << " expected=" << expected[i]
                << std::endl;
            assert(tok == expected[i++]);
        }
        assert(i == expected.size());
    }

}
