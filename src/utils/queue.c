#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "utils.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

extern int32_t queue_recive(int32_t qId, struct Payload *pl, int32_t mType);

extern int32_t queue_send(int32_t qId, const struct Payload *pl);

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
queue_send_error(int32_t qId, const struct ErrorMsg *msg)
{
    struct Payload pl = {};
    pl.mtype = MSG_ERROR;
    memcpy(pl.payload, msg, sizeof(struct ErrorMsg));

    return queue_send(qId, &pl);
}

int32_t
queue_recive_error(int32_t qId, struct ErrorMsg *msg)
{
    struct Payload pl = {};
    ssize_t status = queue_recive(qId, &pl, MSG_ERROR);
    memcpy(msg, pl.payload, sizeof(struct ErrorMsg));

    return status;
}

_Bool
queue_is_empty(int32_t qId)
{
    struct msqid_ds queue = {};
    msgctl(qId, IPC_STAT, &queue);

    return queue.msg_qnum == 0;
}

int32_t
queue_exists(int32_t qId)
{
    struct msqid_ds queue = {};
    return msgctl(qId, IPC_STAT, &queue) >= 0;
}