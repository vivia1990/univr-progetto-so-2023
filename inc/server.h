#ifndef SERVER_H
#define SERVER_H

#include "log.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>

struct Server {
    pid_t pid;
    pid_t connServicePid;
    int32_t connQueueId;
    uint32_t currentPlayer; // index
    struct Logger *logger;
    struct Client *players[2];
    _Bool inGame;
    _Bool isListneningForConnection;
};

struct Client {
    pid_t pid;
    int32_t queueId;
    uint32_t timeoutCounter;
    const char *playerName;
    _Bool winner;
};

struct Server *get_server();

int32_t init_server(struct Server *server);
void print_server(struct Server *server);
int32_t down_server(struct Server *server);

#endif