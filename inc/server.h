#ifndef SERVER_H
#define SERVER_H

#include "log.h"
#include "messages.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>

struct Server {
    pid_t pid;
    pid_t connServicePid;
    int32_t connQueueId;
    int32_t playerCounter;
    uint32_t currentPlayer; // index
    _Bool inGame;
    _Bool isListneningForConnection;
    struct Logger *logger;
    struct Client *players[2];
    int32_t connServicePipe[2];
};

struct Client {
    pid_t pid;
    int32_t queueId;
    uint32_t timeoutCounter;
    _Bool winner;
    char playerName[64];
};

struct Server *get_server();

int32_t init_server(struct Server *server);
int32_t down_server(struct Server *server);

void print_server(struct Server *server);

#endif