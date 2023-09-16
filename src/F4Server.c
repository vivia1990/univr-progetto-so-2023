#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "server.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    struct ServerArgs args = {.columns = 3, .rows = 3};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {0};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);
    LOG_INFO("Server started, pid: %d", server->pid);

    conn_init_manager(server, &args);
    if (conn_resume_listening(server->connMng) < 0) {
        LOG_ERROR("Errore resume connection manager", "")
    }

    LOG_INFO("waiting for players.., connectionManagerPid: "
             "%d\nConnectionQueueId: %d",
             server->connMng->connServicePid, server->connMng->connQueueId);

    add_clients(server, &args);
    LOG_INFO("player connected game starts in few seconds ", "");
    server->connMng->inGame = true;
    conn_resume_listening(server->connMng);

    server_loop(server);

    // log players;

    print_server(server);
    print_game_field(server->gameSettings);

    down_server(server);

    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}