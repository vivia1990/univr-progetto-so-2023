#include "log.h"
#include "messages.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

int32_t
connect(int32_t connQId, const char *playerName)
{
    struct ClientConnectionRequest req = {.clientPid = getpid(),
                                          .typeResp = getpid()};

    struct Payload pl = {.mtype = MSG_CONNECTION};
    memcpy(req.playerName, playerName, strlen(playerName));
    memcpy(pl.payload, &req, sizeof(struct ClientConnectionRequest));
    msgsnd(connQId, &pl, MSG_SIZE(Payload), IPC_NOWAIT);

    msgrcv(connQId, &pl, MSG_SIZE(Payload), req.clientPid, 0660);

    struct ServerConnectionResponse *resp =
        (struct ServerConnectionResponse *)pl.payload;

    LOG_INFO("Connesso %s, queueId: %d", playerName, resp->queueId);

    return 1;
}

int
main(int argc, char const *argv[])
{
    if (argc != 3) {
        LOG_ERROR("Errore argomenti", "");
    }

    if (connect(atoi(argv[1]), argv[2]) < 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}