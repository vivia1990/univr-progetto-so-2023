#ifndef MESSAGES_H
#define MESSAGES_H

#include <fcntl.h>
#include <stdint.h>

#define MSG_CONNECTION 100
#define MSG_GAME 200
#define MSG_GAME_START 220
#define MSG_ERROR 50

#define MSG_SIZE(strct) (sizeof(struct strct) - sizeof(int64_t))

struct Payload {
    int64_t mtype;
    char payload[256];
};

// MSG_CONNECTION
struct ClientConnectionRequest {
    int32_t typeResp;
    pid_t clientPid;
    char playerName[64];
};

// MSG_CONNECTION
struct ServerConnectionResponse {
    int32_t queueId;
};

// MSG_GAME MSG_GAME_START
struct ServerGameResponse {
    uint32_t row;    // row to update
    uint32_t column; // column to update
    _Bool winner;
    _Bool endGame;
};

// MSG_GAME MSG_GAME_START
struct ClientGameRequest {
    int32_t move;
    _Bool restartMatch; // after winning
};

// MSG_ERROR
struct ErrorMsg {
    int32_t errorCode;
    char errorMsg[100];
};

#endif