#include "connection_manager.h"
#include "log.h"
#include "server.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

static int32_t
conn_loop(struct Server *server)
{

    server->isListneningForConnection = true;
    LOG_INFO("connection loop start", "")
    while (1) {
        msgrcv(server->connQueueId, NULL, sizeof(char), 0, 0660);
    }

    return 1;
}

int32_t
conn_create_manager(struct Server *server)
{
    ssize_t qId = msgget(IPC_PRIVATE, 0660); // S_IRUSR | S_IWUSR
    if (qId < 0) {
        PANIC("errore creazione queue", 0, EXIT_FAILURE)
    }

    server->connQueueId = qId;

    pid_t manager = fork();
    if (manager == 0) {
        kill(getpid(), SIGSTOP);
        conn_loop(server);
        LOG_ERROR("Connection manager terminato inaspettatamente", "")
        exit(EXIT_FAILURE);
    }
    waitpid(manager, NULL, WSTOPPED);

    server->isListneningForConnection = false;
    server->connServicePid = manager;

    return 1;
}

int32_t
conn_pause_listening(struct Server *server)
{
    if (!server->isListneningForConnection) {
        return 0;
    }

    server->isListneningForConnection = false;

    return kill(server->connServicePid, SIGSTOP);
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

    ssize_t status = msgctl(server->connQueueId, IPC_RMID, NULL);
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
