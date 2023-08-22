#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "server.h"
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    struct ServerArgs args = {.columns = 7, .rows = 5};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);
    LOG_INFO("Server started, pid: %d", server->pid);

    conn_create_manager(server);
    if (conn_resume_listening(server) < 0) {
        LOG_ERROR("Errore resume connection manager", "")
    }

    LOG_INFO("waiting for players.., connectionManagerPid: "
             "%d\nConnectionQueueId: %d",
             server->connServicePid, server->connQueueId);
    waitpid(server->connServicePid, NULL, WSTOPPED);

    LOG_INFO("player connected game starts in few seconds ", "");
    struct Client clients[2];
    const size_t size = sizeof(struct Client);
    ssize_t bt = read(server->connServicePipe[0], clients, size * 2);

    server->players[0] = memcpy(malloc(size), clients, size);
    server->players[1] = memcpy(malloc(size), clients + 1, size);
    server->playerCounter = 2;
    server->players[0]->symbol = args.symbols[0];
    server->players[1]->symbol = args.symbols[1];
    assert(bt == size * 2);

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