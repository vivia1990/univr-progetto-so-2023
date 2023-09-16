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
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

int32_t
test_game_draw()
{
    struct ServerArgs args = {.columns = 2, .rows = 2};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};
    struct ConnectionManager connMng = {0};

    struct Server *server = get_server();
    server->connMng = &connMng;
    init_server(server, &game, &args);

    if (pipe(server->connMng->connServicePipe) < 0) {
        down_server(server);
        PANIC("Errore creazione pipe connection", EXIT_FAILURE, "")
    }

    LOG_INFO("Server started, pid: %d", server->pid);

    struct ClientConnectionRequest req = {};
    req.clientPid = 1;
    strcpy(req.playerName, "camper");
    req.typeResp = 1;

    struct Client *client = create_client(&req, 'X');
    write(server->connMng->connServicePipe[1], client, sizeof(struct Client));
    free(client);

    req.clientPid = 2;
    strcpy(req.playerName, "campisi");
    req.typeResp = 2;

    client = create_client(&req, 'O');
    write(server->connMng->connServicePipe[1], client, sizeof(struct Client));
    free(client);

    add_clients(server, &args);
    sigset_t signals;
    sigfillset(&signals);
    sigdelset(&signals, SIGINT);
    if (sigprocmask(SIG_SETMASK, &signals, NULL) < 0) {
        LOG_ERROR("Errore set procmask server", "")
    }
    pid_t child = fork();
    if (child == 0) {
        server_init_signals();
        kill(getpid(), SIGSTOP);
        remove("./test/test_server.log");
        ssize_t fd = open("./test/test_server.log", O_CREAT | O_RDWR, 0660);
        dup2(fd, STDOUT_FILENO);
        server_loop(server, 0);
        game_destruct(server->gameSettings);
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

    waitpid(child, NULL, WSTOPPED);
    LOG_INFO("invio segnali", "")
    kill(child, SIGCONT);

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_GAME_START) == MSG_GAME_START);
    }

    {
        struct ClientGameRequest req = {.move = 0, .restartMatch = false};
        assert(queue_send_game(state.firstPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);

        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_SERVER_ACK) == MSG_SERVER_ACK);
    }

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp,
                                 MSG_TURN_START) == MSG_TURN_START);

        struct ClientGameRequest req = {.move = 0, .restartMatch = false};
        assert(queue_send_game(state.secondPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);

        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp,
                                 MSG_SERVER_ACK) == MSG_SERVER_ACK);
    }

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_TURN_START) == MSG_TURN_START);
        struct ClientGameRequest req = {.move = 1, .restartMatch = false};
        assert(queue_send_game(state.firstPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);

        assert(queue_recive_game(state.firstPlayer->queueId, &resp, sizeof resp,
                                 MSG_SERVER_ACK) == MSG_SERVER_ACK);
    }

    {
        struct ServerGameResponse resp = {};
        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp,
                                 MSG_TURN_START) == MSG_TURN_START);
        struct ClientGameRequest req = {.move = 1, .restartMatch = false};
        assert(queue_send_game(state.secondPlayer->queueId, &req, sizeof req,
                               MSG_CLIENT_MOVE) == MSG_CLIENT_MOVE);

        assert(queue_recive_game(state.secondPlayer->queueId, &resp,
                                 sizeof resp, MSG_GAME_END) == MSG_GAME_END);

        ssize_t status = queue_recive_game(state.firstPlayer->queueId, &resp,
                                           sizeof resp, MSG_GAME_END);
        assert(status == MSG_GAME_END);
    }

    wait(NULL);

    down_server(server);
    LOG_INFO("END: test_server_multiple_disconnection", "");

    return 1;
}

int
main(int argc, char const *argv[])
{
    test_game_draw();
    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}