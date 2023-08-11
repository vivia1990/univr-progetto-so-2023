#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    init_server(get_server());

    print_server(get_server());
    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}