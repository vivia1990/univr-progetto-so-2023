#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "server.h"
#include "test.h"
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

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

int32_t
test_server_loop()
{
    struct ServerArgs args = {.columns = 7, .rows = 5};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);

    if (pipe(server->connServicePipe) < 0) {
        down_server(server);
        PANIC("Errore creazione pipe connection", EXIT_FAILURE, "")
    }

    LOG_INFO("Server started, pid: %d", server->pid);

    struct ClientConnectionRequest req = {};
    req.clientPid = 1;
    strcpy(req.playerName, "camper");
    req.typeResp = 1;

    struct Client *client = create_client(&req);
    write(server->connServicePipe[1], client, sizeof(struct Client));
    free(client);

    req.clientPid = 2;
    strcpy(req.playerName, "campisi");
    req.typeResp = 2;

    client = create_client(&req);
    write(server->connServicePipe[1], client, sizeof(struct Client));
    free(client);

    add_clients(server, &args);

    pid_t child = fork();
    if (child == 0) {
        remove("./test/test_server.log");
        ssize_t fd = open("./test/test_server.log", O_CREAT | O_RDWR, 0660);
        dup2(fd, STDOUT_FILENO);
        server_loop(server);
        down_server(server);
        exit(EXIT_SUCCESS);
    }

    struct {
        struct Client *firstPlayer;
        struct Client *secondPlayer;
    } state = {
        .firstPlayer = server->players[0],
        .secondPlayer = server->players[1],
    };
    print_server(server);

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_GAME_START) == MSG_GAME_START);
    }

    {
        struct ClientGameRequest req = {.move = 4, .restartMatch = false};
        assert(queue_send_game(state.firstPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);
    }

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_SERVER_ACK) == MSG_SERVER_ACK);
        assert(resp.column == 4);
    }

    // second player
    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp,
                                 MSG_TURN_START) == MSG_TURN_START);
    }

    {
        struct ClientGameRequest req = {.move = 2, .restartMatch = false};
        assert(queue_send_game(state.secondPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);
    }

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp,
                                 MSG_SERVER_ACK) == MSG_SERVER_ACK);
        assert(resp.column == 2);
    }

    kill(child, SIGKILL);
    wait(NULL);
    down_server(server);

    return 1;
}

int
main(int argc, char const *argv[])
{

    // test_init_server();
    test_server_loop();

    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}