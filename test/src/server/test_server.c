#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "server.h"
#include "test.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t
test_init_server()
{
    struct ServerArgs args = {.columns = 7, .rows = 5};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);
    LOG_INFO("Server started, pid: %d", server->pid);

    print_server(server);
    print_game_field(server->gameSettings);

    down_server(server);

    TEST_PASSED("test_init_server", "")

    return 1;
}

int
main(int argc, char const *argv[])
{

    test_init_server();

    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}