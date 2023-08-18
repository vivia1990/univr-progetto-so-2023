#include "connection_manager.h"
#include "log.h"
#include "messages.h"
#include "server.h"
#include "utils.h"
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

static int32_t
conn_loop(struct Server *server)
{

    LOG_INFO("connection loop start", "")

    int32_t playerCounter = 0;
    struct ClientRequest req = {};
    struct ServerResponse resp = {};
    const int32_t servConnQId = server->connQueueId;
    const size_t sizeReq = sizeof(struct ClientRequest) - sizeof(int64_t);
    const size_t sizeResp = sizeof(struct ServerResponse) - sizeof(int64_t);
    while (1) {
        msgrcv(servConnQId, &req, sizeReq, MSG_CONNECTION, 0660);
        const struct Client *client = create_client(&req);
        if (!client) {
            LOG_ERROR("Errore aggiunta client", "")
            continue;
        }

        resp.mtype = req.typeResp;
        resp.queueId = client->queueId;
        msgsnd(req.typeResp, &resp, sizeResp, IPC_NOWAIT);
        LOG_INFO("Giocatore %s connesso, qId; %d", client->playerName,
                 client->queueId);
        write(server->connServicePipe[1], client, sizeof(struct Client));
        free((struct Client *)client);

        if (++playerCounter == 2) {
            playerCounter = 0;
            // close(server->connServicePipe[1]);
            errno = 0;
            assert(kill(getpid(), SIGSTOP) >= 0);
            perror("\n");
        }
    }

    return 1;
}

int32_t
conn_create_manager(struct Server *server)
{
    server->connQueueId = create_queue();
    if (pipe(server->connServicePipe) < 0) {
        down_server(server);
        PANIC("Errore creazione pipe connection", EXIT_FAILURE, "")
    }

    pid_t manager = fork();
    if (manager == 0) {
        assert(close(server->connServicePipe[0]) >= 0);
        kill(getpid(), SIGSTOP);
        conn_loop(server);

        LOG_ERROR("Connection manager terminato inaspettatamente", "")
        exit(EXIT_FAILURE);
    }

    assert(close(server->connServicePipe[1]) >= 0);
    server->isListneningForConnection = false;
    server->connServicePid = manager;

    waitpid(manager, NULL, WSTOPPED);

    return 1;
}

int32_t
conn_pause_listening(struct Server *server)
{
    if (!server->isListneningForConnection) {
        return 0;
    }

    server->isListneningForConnection = false;

    return kill(getpid(), SIGSTOP);
}

int32_t
conn_resume_listening(struct Server *server)
{
    if (server->isListneningForConnection) {
        return 0;
    }

    server->isListneningForConnection = true;

    return kill(server->connServicePid, SIGCONT);
}

int32_t
conn_remove_manager(struct Server *server)
{

    LOG_INFO("Rimozione manager...", "")

    ssize_t status = remove_queue(server->connQueueId);
    if (status < 0) {
        LOG_ERROR("Errore eliminazione connection queue, key: %d",
                  server->connQueueId)

        return status;
    }

    status = kill(server->connServicePid, SIGTERM);
    if (status < 0) {
        LOG_ERROR("Errore terminazione connection manager, pid: %d",
                  server->connServicePid)

        return status;
    }

    return 1;
}
