#include "queue_api.h"
#include "log.h"
#include "messages.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/msg.h>

static int32_t
queue_send(int32_t qId, const struct Payload *pl)
{
    ssize_t status = 0;
    do {
        errno = 0;
        status = msgsnd(qId, pl, MSG_SIZE(Payload), IPC_NOWAIT);
    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}

static int32_t
queue_recive(int32_t qId, struct Payload *pl, uint32_t mType)
{
    ssize_t status = 0;
    do {
        errno = 0;
        status = msgrcv(qId, pl, MSG_SIZE(Payload), mType, 0660);
    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}

int32_t
queue_send_game(int32_t qId, const void *msg, size_t msgSize,
                int32_t messageType)
{
    struct Payload pl = {.mtype = messageType};
    memcpy(pl.payload, msg, msgSize);

    return queue_send(qId, &pl);
}

int32_t
queue_recive_game(int32_t qId, void *msg, size_t msgSize, int32_t messageType)
{
    struct Payload pl = {};
    ssize_t status = queue_recive(qId, &pl, messageType);
    memcpy(msg, pl.payload, msgSize);

    return status;
}

int32_t
queue_send_connection(int32_t qId, const void *msg, size_t msgSize,
                      uint32_t msgType)
{
    struct Payload pl = {};
    pl.mtype = msgType;
    memcpy(pl.payload, msg, msgSize);

    return queue_send(qId, &pl);
}

int32_t
queue_recive_connection(int32_t qId, void *msg, size_t msgSize,
                        uint32_t msgType)
{
    struct Payload pl = {};
    ssize_t status = queue_recive(qId, &pl, msgType);
    memcpy(msg, pl.payload, msgSize);

    return status;
}

int32_t
queue_send_error(int32_t qId, const void *msg, size_t msgSize)
{
    struct Payload pl = {};
    pl.mtype = MSG_ERROR;
    memcpy(pl.payload, msg, msgSize);

    return queue_send(qId, &pl);
}