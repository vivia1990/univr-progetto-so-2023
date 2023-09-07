
#include "client.h"
#include "game.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int32_t
connect_to_server(struct ClientArgs *args, struct Client *client)
{
    struct ClientConnectionRequest req = {.clientPid = getpid(),
                                          .typeResp = getpid()};

    if (queue_send_connection(args->connQueueId, &req, sizeof req,
                              MSG_CONNECTION) < 0) {
        LOG_ERROR("Errore inivio msg per connessione, qId: %d",
                  args->connQueueId)
    }

    struct ServerConnectionResponse resp = {};
    queue_recive_connection(args->connQueueId, &resp, sizeof resp,
                            req.clientPid);

    client->queueId = resp.queueId;
    client->serverPid = resp.serverPid;
    client->signalDisconnection = resp.disconnectionSignal;
    client->gameSettings->field->columns = resp.fieldColumns;
    client->gameSettings->field->rows = resp.fieldRows;

    LOG_INFO("Connesso %s, queueId: %d", client->playerName, resp.queueId);

    return 1;
}

int32_t
init_client(struct Client *client, const struct ClientArgs *args)
{
    strcpy(client->playerName, args->playerName);
    client->isBot = args->isBot;
    client->pid = getpid();

    return 1;
}

int32_t
parse_client_args(size_t argc, char const **argv, struct ClientArgs *args)
{
    if (argc != 3) {
        LOG_ERROR("Errore argomenti", "");
        return -1;
    }

    args->connQueueId = atoi(argv[1]);
    args->playerName = argv[2];
    args->isBot = false; // todo

    return 1;
}

static const char *
get_bool_lab(_Bool var)
{
    return var ? "true" : "false";
}

void
print_client(struct Client *client)
{
    const char *const lab = "\n+--------------------------------------+\n";
    write(STDOUT_FILENO, lab, strlen(lab));

    LOG_INFO("Client", "");
    puts("----------------------------------------");
    printf("\tPid: %d\n\tPlayerName: %s"
           "\n\tQueueId: %d\n\tisBot: "
           "%s\n\tDisconnectionSignal: %d\n",
           client->pid, client->playerName, client->queueId,
           get_bool_lab(client->isBot), client->signalDisconnection);

    puts("+--------------------------------------+\n");
    fflush(stdout);
}