#ifndef MESSAGES_H
#define MESSAGES_H

#include <fcntl.h>
#include <stdint.h>

#define MSG_ERROR 50
#define MSG_CONNECTION 100
#define MSG_GAME_END 170
#define MSG_SERVER_ACK 180
#define MSG_CLIENT_MOVE 190
#define MSG_TURN_START 200
#define MSG_GAME_START 220

#define MSG_SIZE(strct) (sizeof(struct strct) - sizeof(int64_t))

// MSG_CONNECTION
struct ClientConnectionRequest {
    int32_t typeResp;
    pid_t clientPid;
    char playerName[64];
};

// MSG_CONNECTION
struct ServerConnectionResponse {
    int32_t queueId;
    int32_t disconnectionSignal; // SIGUSR1 SIGUSR2
    int32_t fieldRows;
    int32_t fieldColumns;
    char symbol;
    char opponentSymbol;
    pid_t serverPid;
};

// MSG_GAME MSG_GAME_START
struct ServerGameResponse {
    uint32_t row;    // row to update
    uint32_t column; // column to update
    _Bool winner;
    _Bool draw;
    _Bool endGame;
};

// MSG_GAME MSG_GAME_START
struct ClientGameRequest {
    int32_t move;       // indice colonna campo di gioco
    _Bool restartMatch; // after winning
};

// MSG_ERROR
struct ErrorMsg {
    int32_t errorCode;
    char errorMsg[100];
};

#endif