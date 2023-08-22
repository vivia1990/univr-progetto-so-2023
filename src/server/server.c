#include "server.h"
#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "utils.h"
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int32_t
init_server(struct Server *server, struct GameSettings *gameSettings,
            struct ServerArgs *args)
{
    server->pid = getpid();
    server->inGame = false;

    struct Logger *logger = get_logger();
    if (log_init(logger, STDOUT_FILENO, STDERR_FILENO) < 0) {
        LOG_ERROR("Errore inizializzazione log", "")
        return -1;
    }

    server->logger = logger;
    game_init(gameSettings, args->rows, args->columns);
    server->gameSettings = gameSettings;

    return 1;
}

struct Client *
create_client(struct ClientRequest *request)
{

    struct Client *client = malloc(sizeof(struct Client));
    client->pid = request->clientPid;
    strcpy(client->playerName, request->playerName);
    client->timeoutCounter = 0;
    client->winner = false;

    ssize_t qId = create_queue();
    if (qId == -1) {
        return NULL;
    }
    client->queueId = qId;

    return client;
}

int32_t
down_server(struct Server *server)
{
    conn_remove_manager(server);
    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        if (!client) {
            continue;
        }
        if (remove_queue(client->queueId) < 0) {
            LOG_ERROR("Impossibile rimuovere msgqueue: %d, client %s",
                      client->queueId, client->playerName);
        }

        free(client);
    }

    close(server->connServicePipe[0]);
    close(server->connServicePipe[1]);

    game_destruct(server->gameSettings);

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

    puts("\nClients:");
    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        printf("\tPid: %d\n\tPlayerName: %s\n\tQueueId: "
               "%d\n\tTimeoutCounter:  %d\n\twinner %d\n\n\tsymbol %c\n\n",
               client->pid, client->playerName, client->queueId,
               client->timeoutCounter, client->winner, client->symbol);
    }
    puts("+--------------------------------------+\n");
    fflush(stdout);
}