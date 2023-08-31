#ifndef SERVER_H
#define SERVER_H

typedef struct GameSettings GameSettings;

#include "game.h"
#include "log.h"
#include "messages.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>

struct ServerArgs {
    int32_t rows;
    int32_t columns;
    char symbols[2];
};

struct Server {
    pid_t pid;
    pid_t connServicePid;
    int32_t connQueueId;
    int32_t playerCounter;
    uint32_t currentPlayer; // index
    _Bool inGame;
    _Bool isListneningForConnection;
    _Bool wasCtrlCPressed; // verifica se prima del prossimo messaggio ctrl-c è
                           // stato premuto
    _Bool disconnectionHappened;
    uint32_t disconnectionCounter;
    struct Logger *logger;
    struct Client *players[2];
    int32_t connServicePipe[2];
    struct GameSettings *gameSettings;
};

struct Client {
    pid_t pid;
    int32_t queueId;
    uint32_t timeoutCounter;
    char symbol;
    _Bool winner;
    _Bool disconnected;
    char playerName[64];
};

struct Server *get_server();

int32_t server_init_signals();

int32_t init_server(struct Server *server, struct GameSettings *gameSettings,
                    struct ServerArgs *args);

int32_t server_loop(struct Server *server);

int32_t add_clients(struct Server *server, struct ServerArgs *args);

int32_t down_server(struct Server *server);

void print_server(struct Server *server);

#endif