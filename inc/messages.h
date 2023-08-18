#ifndef MESSAGES_H
#define MESSAGES_H

#include <fcntl.h>
#include <stdint.h>

#define MSG_CONNECTION 100

struct ClientRequest {
    int64_t mtype;
    int32_t typeResp;
    pid_t clientPid;
    char playerName[64];
};

struct ServerResponse {
    int64_t mtype;
    int32_t queueId;
};

#endif