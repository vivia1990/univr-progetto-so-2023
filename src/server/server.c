#include "server.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int32_t
init_server(struct Server *server)
{
    server->pid = getpid();
    server->inGame = false;

    struct Logger *logger = get_logger();
    if (log_init(logger, STDOUT_FILENO, STDERR_FILENO) < 0) {
        LOG_ERROR("Errore inizializzazione log", "")
        return -1;
    }

    server->logger = logger;

    return 1;
}

int32_t
down_server(struct Server *server)
{
    return 0;
}

void
print_server(struct Server *server)
{

    LOG_INFO("Server", "");
    printf("Pid: %d\nConnectionServicePid: %d\nConnectionQueueId: %d\nInGame: "
           "%d\n",
           server->pid, server->connServicePid, server->connQueueId,
           server->inGame);
}