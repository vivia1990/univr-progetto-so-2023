#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>

char *
int_to_string(int32_t value, char *buff, size_t length)
{
    snprintf(buff, length, "%d", value);
    return buff;
}

ssize_t
create_fifo(const char *path)
{
    unlink(path);
    return mkfifo(path, 0660);
}

ssize_t
create_queue()
{
    ssize_t qId = msgget(IPC_PRIVATE, 0660); // S_IRUSR | S_IWUSR
    if (qId < 0) {
        PANIC("errore creazione queue", 0, EXIT_FAILURE)
        return -1;
    }

    return qId;
}

ssize_t
remove_queue(int32_t queueId)
{
    return msgctl(queueId, IPC_RMID, NULL);
}