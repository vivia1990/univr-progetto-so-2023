#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "server.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/msg.h>

int32_t
queue_recive(int32_t qId, struct Payload *pl, uint32_t mType)
{
    ssize_t status = 0;
    struct Server *server = get_server();
    do {
        errno = 0;
        status = msgrcv(qId, pl, MSG_SIZE(Payload), mType, 0660);

        if (errno == EINTR && server->disconnectionHappened) {
            return status;
        }
    } while (errno == EINTR);

    if (status < 0) {
        LOG_ERROR("Errore invio payload queue_api", "");
        return status;
    }

    return pl->mtype;
}