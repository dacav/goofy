#include "server.h"

int main(int argc, char **argv)
{
    spg::Server srv = spg::Server::parse_addr(argv[1], 8070, 8);
}
