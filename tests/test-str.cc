#include <cassert>
#include "util/str.h"

using namespace goofy;

int main(int argc, char** argv)
{
    const std::string hello("   hello world");

    util::StrRef ref(hello);
    assert(std::string(ref) == hello);
}
