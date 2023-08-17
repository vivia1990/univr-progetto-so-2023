#include "server.h"
#include "connection_manager.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
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
    conn_remove_manager(server);
    return 0;
}

void
print_server(struct Server *server)
{
    const char *const lab = "\n+--------------------------------------+\n";
    write(STDOUT_FILENO, lab, strlen(lab));
    LOG_INFO("Server", "");
    puts("----------------------------------------");
    printf("\tPid: %d\n\tConnectionServicePid: %d\n\tConnectionQueueId: "
           "%d\n\tInGame: "
           "%d\n\tIsListeningForConnection %d\n",
           server->pid, server->connServicePid, server->connQueueId,
           server->inGame, server->isListneningForConnection);
    puts("+--------------------------------------+\n");
    fflush(stdout);
}