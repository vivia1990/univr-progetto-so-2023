

#include "log.h"
#include <fcntl.h>
#include <game.h>
#include <stdint.h>

struct Client {
    pid_t pid;
    pid_t serverPid;
    int32_t queueId;
    uint32_t timeoutCounter;
    uint32_t signalDisconnection;
    _Bool isBot;
    char symbol;
    char playerName[64];
    struct GameSettings *gameSettings;
};

struct ClientArgs {
    int32_t connQueueId;
    _Bool isBot;
    const char *playerName;
};

struct Client *get_client();

int32_t parse_client_args(size_t argc, char const **argv,
                          struct ClientArgs *args);

int32_t connect_to_server(struct ClientArgs *args, struct Client *client);

int32_t init_client(struct Client *client, const struct ClientArgs *args);

void print_client(struct Client *client);