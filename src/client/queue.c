#include "client.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "utils.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

int32_t
queue_recive(int32_t qId, struct Payload *pl, int32_t mType)
{
    ssize_t status = 0;
    do {
        errno = 0;
        status = msgrcv(qId, pl, MSG_SIZE(Payload), mType, 0660);

        if (status < 0 && errno != EINTR) {
            LOG_ERROR("Server non più raggiungibile, %d",
                      get_client()->serverPid);
            down_client(get_client());
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
            LOG_ERROR("Errore non gestito", "")
            down_client(get_client());
        }
    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}