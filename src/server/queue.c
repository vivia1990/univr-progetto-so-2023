#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "server.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

int32_t
queue_recive(int32_t qId, struct Payload *pl, int32_t mType)
{
    ssize_t status = 0;
    struct Server *server = get_server();
    do {
        errno = 0;
        alarm(SERVER_TIMEOUT_SECONDS);
        status = msgrcv(qId, pl, MSG_SIZE(Payload), mType, 0660);
        LOG_INFO("errno %d, disconnection: %d\ntimeout: %d\n", errno,
                 server->disconnectionHappened, server->timeoutHappened)
        if (errno == EINTR &&
            (server->disconnectionHappened || server->timeoutHappened)) {
            return status;
        }
        alarm(0);

    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}