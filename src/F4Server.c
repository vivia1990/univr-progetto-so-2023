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

int32_t
server_parse_args(int argc, const char *argv[], struct ServerArgs *args)
{
    if (argc != 5) {
        printf("Utilizzo: %s <righe> <colonne> <player1> <player2>\n", argv[0]);
        return -1;
    }

    args->rows = atoi(argv[1]);
    args->columns = atoi(argv[2]);

    if (args->rows < 4 || args->columns < 4) {
        printf("Il campo da gioco deve essere almeno 4x4\n");
        return -1;
    }

    if (strlen(argv[3]) != 1 || strlen(argv[4]) != 1) {
        printf("Simboli devono essere caratteri ascii\n");
        return -1;
    }

    if (argv[3][0] > 127 || argv[4][0] > 127) {
        printf("Simboli devono essere caratteri ascii\n");
        return -1;
    }

    args->symbols[0] = argv[3][0];
    args->symbols[1] = argv[4][0];

    return 0;
}

int
main(int argc, char const *argv[])
{
    struct ServerArgs args = {};
    if (server_parse_args(argc, argv, &args) < 0) {
        return EXIT_FAILURE;
    }

    struct GameField gf = {0};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);
    LOG_INFO("Server started, pid: %d", server->pid);

    conn_init_manager(server, &args);
    if (conn_resume_listening(server->connMng) < 0) {
        LOG_ERROR("Errore resume connection manager", "")
    }

    LOG_INFO("waiting for players..\n", "")
    LOG_INFO("Connection Service disponibile su ==> %d\n",
             server->connMng->connQueueId);

    add_clients(server, &args);
    LOG_INFO("player connected game starts in few seconds ", "");
    server->connMng->inGame = true;
    conn_resume_listening(server->connMng);

    uint32_t firstPlayer = 0;
    while (1) {
        const int32_t status = server_loop(server, firstPlayer);

        if (status == 0 || status == 1) {
            firstPlayer = !status;
        }
        else {
            break;
        }
    }

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