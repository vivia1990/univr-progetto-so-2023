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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}

int32_t
test_connection_server()
{
    LOG_INFO("test_connection_server", "")
    struct ServerArgs args = {.columns = 7, .rows = 5};
    args.symbols[0] = 'O';
    args.symbols[1] = 'X';

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    struct Server *server = get_server();
    init_server(server, &game, &args);
    conn_init_manager(server);
    print_server(server);

    if (conn_resume_listening(server->connMng) < 0) {
        LOG_ERROR("Errore resume connection manager", "")
    }

    LOG_INFO("waiting for players.., connectionManagerPid: "
             "%d\nConnectionQueueId: %d",
             server->connMng->connServicePid, server->connMng->connQueueId);

    pid_t client = fork();
    if (client == 0) {
        int32_t connQId = server->connMng->connQueueId;
        sigset_t signals;
        sigfillset(&signals);
        sigprocmask(SIG_SETMASK, &signals, NULL);

        {
            struct ClientConnectionRequest req = {.clientPid = 1,
                                                  .typeResp = 1};

            memcpy(req.playerName, "mich", sizeof "mich");
            queue_send_connection(connQId, &req, sizeof req, MSG_CONNECTION);

            struct ServerConnectionResponse resp = {};
            queue_recive_connection(connQId, &resp, sizeof resp, req.clientPid);

            LOG_INFO("Connesso queueId: %d, serverPid: %d", resp.queueId,
                     resp.serverPid);
        }

        {
            struct ClientConnectionRequest req = {.clientPid = 2,
                                                  .typeResp = 2};

            memcpy(req.playerName, "mich", sizeof "vivi");
            queue_send_connection(connQId, &req, sizeof req, MSG_CONNECTION);

            struct ServerConnectionResponse resp = {};
            queue_recive_connection(connQId, &resp, sizeof resp, req.clientPid);

            LOG_INFO("Connesso queueId: %d, serverPid: %d", resp.queueId,
                     resp.serverPid);
        }
        exit(EXIT_SUCCESS);
    }
    wait(NULL);
    assert(add_clients(server, &args) == 2);
    server->connMng->inGame = true;
    conn_resume_listening(server->connMng);

    {
        int32_t connQId = server->connMng->connQueueId;
        struct ClientConnectionRequest req = {.clientPid = 3, .typeResp = 3};

        memcpy(req.playerName, "mich", sizeof "mich");
        queue_send_connection(connQId, &req, sizeof req, MSG_CONNECTION);

        struct ErrorMsg err = {};
        queue_recive_error(connQId, &err);
        assert(err.errorCode == 503);
        assert(strcmp(err.errorMsg, "Partita in corso, riprovare più tardi") ==
               0);
    }

    down_server(server);

    TEST_PASSED("test_connection_server", "")

    return 1;
}

int
main(int argc, char const *argv[])
{
    test_connection_server();
    return EXIT_SUCCESS;
}