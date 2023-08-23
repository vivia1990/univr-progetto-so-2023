#include "connection_manager.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "server.h"
#include "utils.h"
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

static int32_t
conn_loop(struct Server *server)
{

    LOG_INFO("connection loop start", "")

    struct ClientConnectionRequest req = {};
    struct ServerConnectionResponse resp = {};
    const int32_t servConnQId = server->connQueueId;

    int32_t playerCounter = 0;
    while (1) {
        queue_recive_connection(servConnQId, &req, sizeof req, MSG_CONNECTION);

        const struct Client *client = create_client(&req);
        if (!client) {
            LOG_ERROR("Errore aggiunta client", "")
            continue;
        }

        resp.queueId = client->queueId;
        queue_send_connection(servConnQId, &resp, sizeof resp, req.clientPid);
        LOG_INFO("Giocatore %s, PID: %d connesso, qId; %d", client->playerName,
                 client->pid, client->queueId);

        write(server->connServicePipe[1], client, sizeof(struct Client));
        free((struct Client *)client);

        if (++playerCounter == 2) {
            playerCounter = 0;
            assert(kill(getpid(), SIGSTOP) >= 0);
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
        sigset_t signals;
        sigfillset(&signals);
        sigprocmask(SIG_SETMASK, &signals, NULL);

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

    status = kill(server->connServicePid, SIGKILL);
    if (status < 0) {
        LOG_ERROR("Errore terminazione connection manager, pid: %d",
                  server->connServicePid)

        return status;
    }

    return 1;
}
