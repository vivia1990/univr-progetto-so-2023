#include "log.h"
#include "messages.h"
#include "queue_api.h"
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

    queue_send_connection(connQId, &req, sizeof req, MSG_CONNECTION);

    struct ServerConnectionResponse resp = {};

    queue_recive_connection(connQId, &resp, sizeof resp, req.clientPid);

    LOG_INFO("Connesso %s, queueId: %d", playerName, resp.queueId);

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