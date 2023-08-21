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
    struct ClientRequest req = {
        .clientPid = getpid(), .typeResp = getpid(), .mtype = MSG_CONNECTION};

    memcpy(req.playerName, playerName, strlen(playerName));
    const size_t sizeReq = sizeof(struct ClientRequest) - sizeof(int64_t);
    msgsnd(connQId, &req, sizeReq, IPC_NOWAIT);

    struct ServerResponse resp = {};
    const size_t sizeResp = sizeof(struct ServerResponse) - sizeof(int64_t);
    msgrcv(connQId, &resp, sizeResp, req.clientPid, 0);

    printf("Connesso %s, queueId: %d", playerName, resp.queueId);

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