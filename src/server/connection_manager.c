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
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

static int32_t
conn_loop(struct ConnectionManager *manager)
{

    LOG_INFO("connection loop start", "")

    struct ClientConnectionRequest req = {};
    struct ServerConnectionResponse resp = {};
    const int32_t servConnQId = manager->connQueueId;
    const struct ErrorMsg gameStartedErr = {
        .errorCode = 503, .errorMsg = "Partita in corso, riprovare più tardi"};

    int32_t playerCounter = 0;
    while (1) {
        queue_recive_connection(servConnQId, &req, sizeof req, MSG_CONNECTION);
        if (manager->inGame) {
            LOG_INFO("Server pieno: client %s ha provato a connettersi",
                     req.playerName)
            queue_send_error(servConnQId, &gameStartedErr);
            continue;
        }

        const struct Client *client = create_client(&req);
        if (!client) {
            LOG_ERROR("Errore aggiunta client", "")
            continue;
        }

        resp.serverPid = getppid();
        resp.queueId = client->queueId;
        resp.disconnectionSignal = SIGUSR1 + 2 * playerCounter;
        queue_send_connection(servConnQId, &resp, sizeof resp, req.clientPid);
        LOG_INFO("Giocatore %s, PID: %d connesso, qId; %d", client->playerName,
                 client->pid, client->queueId);

        write(manager->connServicePipe[1], client, sizeof(struct Client));
        free((struct Client *)client);

        if (++playerCounter == 2) {
            playerCounter = 0;
            conn_pause_listening(manager);
        }
    }

    return 1;
}

int32_t
conn_init_manager(struct Server *server)
{
    int32_t shm = shmget(IPC_PRIVATE, sizeof(struct ConnectionManager), 0660);
    if (shm < 0) {
        LOG_ERROR("Errore creazione shared memory connection manager", "")
        return -1;
    }

    struct ConnectionManager *manager = shmat(shm, NULL, 0);
    if (!manager) {
        LOG_ERROR("Errore attach connection manager shared memory, id: %d", shm)
        return -1;
    }
    server->connMng = manager;

    manager->connShm = shm;
    manager->inGame = false;
    manager->connQueueId = create_queue();
    if (pipe(manager->connServicePipe) < 0) {
        down_server(server);
        PANIC("Errore creazione pipe connection", EXIT_FAILURE, "")
    }

    const pid_t worker = fork();
    if (worker == 0) {
        sigset_t signals;
        sigfillset(&signals);
        sigprocmask(SIG_SETMASK, &signals, NULL);

        struct ConnectionManager *manager = shmat(shm, NULL, 0);
        manager->connServicePid = getpid();

        assert(close(manager->connServicePipe[0]) >= 0);
        manager->isListneningForConnection = true;

        conn_pause_listening(manager);
        conn_loop(manager);

        LOG_ERROR("Connection manager terminato inaspettatamente", "")
        exit(EXIT_FAILURE);
    }

    assert(close(manager->connServicePipe[1]) >= 0);

    waitpid(worker, NULL, WSTOPPED);

    return 1;
}

int32_t
conn_pause_listening(struct ConnectionManager *manager)
{
    if (!manager->isListneningForConnection) {
        return 0;
    }

    manager->isListneningForConnection = false;

    return kill(manager->connServicePid, SIGSTOP);
}

int32_t
conn_resume_listening(struct ConnectionManager *manager)
{
    if (manager->isListneningForConnection) {
        return 0;
    }

    manager->isListneningForConnection = true;

    return kill(manager->connServicePid, SIGCONT);
}

int32_t
conn_remove_manager(struct ConnectionManager *manager)
{

    LOG_INFO("Rimozione manager...", "")

    if (manager->connServicePid == 0) {
        return 1;
    }

    ssize_t status = remove_queue(manager->connQueueId);
    if (status < 0) {
        LOG_ERROR("Errore eliminazione connection queue, key: %d",
                  manager->connQueueId)

        return status;
    }

    status = kill(manager->connServicePid, SIGKILL);
    if (status < 0) {
        LOG_ERROR("Errore terminazione connection manager, pid: %d",
                  manager->connServicePid)

        return status;
    }

    status = shmctl(manager->connShm, IPC_RMID, NULL);
    if (status < 0) {
        LOG_ERROR("Errore rimozione connection manager shared memory", "")
        return status;
    }

    close(manager->connServicePipe[0]);
    close(manager->connServicePipe[1]);

    return 1;
}
