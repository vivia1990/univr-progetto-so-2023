#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "server.h"
#include "utils.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
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
        alarm(0);

        if (status < 0 && errno != EINTR) {
            down_server(server);
            PANIC("Errore non gestito", EXIT_FAILURE, "")
        }

        LOG_INFO("errno %d, disconnection: %d\ntimeout: %d\n", errno,
                 server->disconnectionHappened, server->timeoutHappened)
        if (errno == EINTR &&
            (server->disconnectionHappened || server->timeoutHappened)) {
            return status;
        }

    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}

int32_t
queue_send(int32_t qId, const struct Payload *pl)
{
    ssize_t status = 0;
    do {
        errno = 0;
        status = msgsnd(qId, pl, MSG_SIZE(Payload), IPC_NOWAIT);
        if (status < 0 && errno != EINTR) {
            down_server(get_server());
            PANIC("Errore non gestito", EXIT_FAILURE, "")
        }
    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}