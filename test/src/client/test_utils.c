#include "client.h"
#include "queue_api.h"
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char const *argv[])
{
    printf("%d", queue_exists(229433));
    perror("");
    return EXIT_SUCCESS;
}

struct Client *
get_client()
{
    static struct Client client = {0};
    return &client;
}