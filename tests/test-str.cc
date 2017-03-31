#include <vector>
#include <cassert>
#include "util/str.h"

#include <iostream>

using namespace goofy;

namespace
{
    template <typename IntType>
    void test_strto();

    void test_strref();

    void test_tokenizer(
        const std::string testcase,
        std::vector<std::string> expected
    );
}

int main(int argc, char** argv)
{
    test_strto<int8_t>();
    test_strto<uint8_t>();
    test_strto<int16_t>();
    test_strto<uint16_t>();
    test_strto<int32_t>();
    test_strto<uint32_t>();
    //test_strto<int64_t>();
    //test_strto<uint64_t>();

    test_strref();

    test_tokenizer("hello world", {"hello", "world"});
    test_tokenizer("hello", {"hello"});
    test_tokenizer("   hello world   ",
        {"", "", "", "hello", "world", "", "", ""}
    );

    return 0;
}

namespace
{
    template <typename T1, typename T2>
    void assert_equals(const T1& v1, const T2& v2)
    {
        std::cerr << "assert(" << (long long)(v1) << " == " << (long long)(v2) << ")" << std::endl;
        assert(v1 == v2);
    }

    template <typename IntType>
    void test_strto()
    {
        std::cerr << "--- test_strto ---" << std::endl;

        const long long min = std::numeric_limits<IntType>::min();
        const long long max = std::numeric_limits<IntType>::max();
        assert_equals(util::strto<IntType>(std::to_string(min)), min);
        assert_equals(util::strto<IntType>(std::to_string(max)), max);

        try {
            util::strto<IntType>(std::to_string((long long)(max + 1)));
            assert(false);
        }
        catch (Error& e) {
            std::cerr << "Expected error: " << e.what() << std::endl;
        }
        try {
            util::strto<IntType>(std::to_string(min - 1));
            assert(false);
        }
        catch (Error& e) {
            std::cerr << "Expected error: " << e.what() << std::endl;
        }

    }

    void test_strref()
    {
        std::cerr << "--- test_strref ---" << std::endl;
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
