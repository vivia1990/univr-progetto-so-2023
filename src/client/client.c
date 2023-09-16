
#include "client.h"
#include "game.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

struct State {
    struct Client *client;
    struct RenderString *string;
    struct GameField *field;
    uint32_t columns;
};
static int32_t process_error(struct Payload *pl, struct State *state,
                             char *buffer);
static int32_t process_game_end(struct Payload *pl, struct State *state);
static int32_t process_turn_start(struct Payload *pl, struct State *state);
static int32_t process_turn_end(struct Payload *pl, struct State *state);

extern int32_t queue_recive(int32_t qId, struct Payload *pl, int32_t mType);

const char *const labelMessages[] = {
    "==> Congratulazioni, hai vinto!\n",
    "==> Sconfitta :(\n",
    "==> Gioco terminato, Pareggio\n",
    "==> Mossa precedente non valida!\n\n",
    "==> Attesa mossa altro giocatore...\n",
    "==> Ci hai messo troppo, turno perso\n",
    "==> In attesa di altri giocatori\n",
    "==> In attesa della decisione dell'altro giocatore\n",
    "==> Vuoi ricominciare la partita? (Yes/No) ",
    "==> Match Restarting...\n",
};

int32_t
connect_to_server(struct ClientArgs *args, struct Client *client)
{
    struct ClientConnectionRequest req = {.clientPid = getpid(),
                                          .typeResp = getpid()};

    memcpy(req.playerName, args->playerName, strlen(args->playerName));

    if (!queue_exists(args->connQueueId)) {
        LOG_INFO("Message queue inesistente! %d", args->connQueueId);
        return -1;
    }

    if (queue_send_connection(args->connQueueId, &req, sizeof req,
                              MSG_CONNECTION) < 0) {
        PANIC("Server non raggiungibile o numero message queue non "
              "corretto, qId: %d",
              EXIT_FAILURE, args->connQueueId);
    }

    struct ServerConnectionResponse resp = {};
    queue_recive_connection(args->connQueueId, &resp, sizeof resp,
                            req.clientPid);

    client->queueId = resp.queueId;
    client->serverPid = resp.serverPid;
    client->symbol = resp.symbol;
    client->opponentSymbol = resp.opponentSymbol;
    client->signalDisconnection = resp.disconnectionSignal;
    client->gameSettings->field->columns = resp.fieldColumns;
    client->gameSettings->field->rows = resp.fieldRows;

    LOG_INFO("Connesso %s, queueId: %d", client->playerName, resp.queueId);

    return 1;
}

static int32_t
process_error(struct Payload *pl, struct State *state, char *buffer)
{
    struct ErrorMsg errorMsg = {};
    memcpy(&errorMsg, pl->payload, sizeof errorMsg);
    const char *const msgPrefix = "\n==> ";

    const size_t size = strlen(msgPrefix);
    size_t length = size + strlen(errorMsg.errorMsg);
    memcpy(buffer, msgPrefix, size);
    strcpy(buffer + size, errorMsg.errorMsg);

    if (errorMsg.errorCode == 400) {
        buffer[length++] = '\n';
        buffer[length++] = '\n';
        write(STDOUT_FILENO, buffer, length);
        return process_turn_start(pl, state);
    }

    if (errorMsg.errorCode == 408) { // singolo timeout
        buffer[length++] = '\0';
        clear_terminal();
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){buffer}, 1);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return bytes;
    }

    if (errorMsg.errorCode == 450) {
        buffer[length++] = '\n';
        buffer[length++] = '\0';
        clear_terminal();
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){buffer}, 1);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return -3;
    }

    if (errorMsg.errorCode == 600) { // timeout multipli
        buffer[length++] = '\n';
        buffer[length++] = '\0';
        clear_terminal();
        size_t bytes =
            client_render(state->client, state->string,
                          (const char *[2]){buffer, labelMessages[1]}, 2);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return -3;
    }

    if (errorMsg.errorCode == 601) { // disconnessione altro player
        buffer[length++] = '\n';
        buffer[length++] = '\0';
        clear_terminal();
        size_t bytes =
            client_render(state->client, state->string,
                          (const char *[2]){buffer, labelMessages[0]}, 2);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return -5;
    }

    if (errorMsg.errorCode == 602) {
        buffer[length++] = '\n';
        buffer[length++] = '\0';
        clear_terminal();
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){buffer}, 1);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return -8;
    }

    assert(false == true);
}

_Bool
client_confirm_restart(struct Client *client)
{
    char resp[4] = {0};

    while (1) {
        errno = 0;
        read(STDIN_FILENO, resp, 3);
        resp[3] = '\0';

        return tolower(resp[0]) == 'y';
    }

    return false;
}

static int32_t
process_game_end(struct Payload *pl, struct State *state)
{
    struct ServerGameResponse resp = {};
    memcpy(&resp, pl->payload, sizeof resp);
    if (resp.updateField) {
        game_set_point_index(state->field, resp.row, resp.column,
                             state->client->opponentSymbol);
    }

    if (resp.winner) {
        size_t bytes = client_render(
            state->client, state->string,
            (const char *[2]){labelMessages[0], labelMessages[8]}, 2);

        // todo ask rematch
        clear_terminal();
        write(STDOUT_FILENO, state->string->renderData, bytes);
        struct ClientGameRequest req = {0};

        if (client_confirm_restart(state->client)) {
            req.restartMatch = true;
            queue_send_game(state->client->queueId, &req, sizeof req,
                            MSG_CLIENT_MOVE);
            clear_terminal();
            size_t bytes =
                client_render(state->client, state->string,
                              (const char *[1]){labelMessages[9]}, 1);
            write(STDOUT_FILENO, state->string->renderData, bytes);

            return -6;
        }

        req.restartMatch = false;
        queue_send_game(state->client->queueId, &req, sizeof req,
                        MSG_CLIENT_MOVE);
        return -7;
    }

    if (resp.draw) {
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){labelMessages[2]}, 1);
        write(STDOUT_FILENO, state->string->renderData, bytes);

        return -4;
    }

    if (resp.endGame) { // lost
        size_t bytes = client_render(
            state->client, state->string,
            (const char *[2]){labelMessages[1], labelMessages[7]}, 2);
        write(STDOUT_FILENO, state->string->renderData, bytes);
        return -9;
    }
    return 1;
}

int32_t
client_get_move(int32_t maxColumns, struct Client *client)
{
    char number[4] = {0};

    int32_t move = 0;
    stop_file_block(STDIN_FILENO);
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n')
        ;
    resume_file_block(STDIN_FILENO);

    while (1) {
        errno = 0;
        read(STDIN_FILENO, number, 3);
        if (client->timeoutHappened) {
            client->timeoutHappened = false;
            resume_file_block(STDIN_FILENO);

            return -2;
        }

        number[3] = '\0';

        move = atoi(number);
        if (move <= 0 || move > maxColumns) {
            memset(number, 0, 4);
            return -1;
        }

        break;
    }

    return move;
}

static int32_t
process_turn_start(struct Payload *pl, struct State *state)
{
    if (pl->mtype == MSG_TURN_START) {
        struct ServerGameResponse resp = {};
        memcpy(&resp, pl->payload, sizeof resp);

        game_set_point_index(state->field, resp.row, resp.column,
                             state->client->opponentSymbol);
    }
    const char *const askMsg = "==> È il tuo turno\n\tScegli una colonna: ";
    ssize_t bytes = client_render(state->client, state->string,
                                  (const char *[1]){askMsg}, 1);

    write(STDOUT_FILENO, state->string->renderData, bytes);
    int32_t move = 0; // rand() % state->columns;
    while (move <= 0) {
        move = client_get_move(state->columns, state->client);
        if (move == -1) {
            clear_terminal();
            write(STDOUT_FILENO, labelMessages[3], strlen(labelMessages[3]));
            write(STDOUT_FILENO, state->string->renderData, bytes);
            continue;
        }

        if (move == -2) {
            return 0;
        }
    }

    struct ClientGameRequest req = {.move = move - 1, .restartMatch = false};
    queue_send_game(state->client->queueId, &req, sizeof req, MSG_CLIENT_MOVE);

    return bytes - strlen(askMsg);
}

static int32_t
process_turn_end(struct Payload *pl, struct State *state)
{
    struct ServerGameResponse resp = {};
    memcpy(&resp, pl->payload, sizeof resp);

    game_set_point_index(state->field, resp.row, resp.column,
                         state->client->symbol);

    return client_render(state->client, state->string,
                         (const char *[1]){labelMessages[4]}, 1);
}

int32_t
init_render_string(struct RenderString *obj)
{
    obj->renderData = calloc(2048, 1);
    obj->size = 2048;
    obj->length = 0;

    return 1;
}

int32_t
client_render(struct Client *client, struct RenderString *rString,
              const char **messages, size_t nMessages)
{
    size_t totalBytesW = 0;
    char *outString = rString->renderData;
    struct GameField *field = client->gameSettings->field;
    const char *const hudMsg = "Giocatore: %c  ------- Avversario: %c\n\n";
    const char *const padStr = "-----+";
    const size_t sizePadStr = strlen(padStr);

    totalBytesW += snprintf(outString + totalBytesW, rString->size, hudMsg,
                            client->symbol, client->opponentSymbol);

    totalBytesW +=
        snprintf(outString + totalBytesW, rString->size, "%*c", 5, 0x20);

    for (uint32_t i = 0; i < field->columns; i++) {
        totalBytesW += snprintf(outString + totalBytesW, rString->size,
                                "%*d%*c", 4, i + 1, 2, 0x20);
    }
    outString[totalBytesW++] = '\n';

    totalBytesW +=
        snprintf(outString + totalBytesW, rString->size, "%*c", 5, 0x20);

    outString[totalBytesW++] = '*';
    for (size_t i = 0; i < field->columns; i++) {
        memcpy((outString + totalBytesW) + (i * sizePadStr), padStr,
               sizePadStr);
    }
    totalBytesW += field->columns * (sizePadStr);
    outString[totalBytesW - 1] = '*';
    outString[totalBytesW++] = '\n';

    for (size_t i = 0; i < field->rows; i++) {
        totalBytesW += snprintf(outString + totalBytesW, rString->size,
                                "%*ld%*c", 3, i + 1, 2, 0x20);
        for (size_t j = 0; j < field->columns; j++) {
            totalBytesW +=
                snprintf(outString + totalBytesW, rString->size, "|%*c%*c", 3,
                         field->matrix[i][j], -2, 0x20);
        }
        outString[totalBytesW++] = '|';
        outString[totalBytesW++] = '\n';

        totalBytesW +=
            snprintf(outString + totalBytesW, rString->size, "%*c", 5, 0x20);
        outString[totalBytesW++] = '|';
        for (size_t i = 0; i < field->columns; i++) {
            memcpy((outString + totalBytesW) + (i * sizePadStr), padStr,
                   sizePadStr);
        }
        totalBytesW += field->columns * (sizePadStr);

        outString[totalBytesW++] = '\n';
    }
    outString[totalBytesW - 2] = '*';
    outString[totalBytesW - (field->columns * (sizePadStr)) - 2] = '*';

    if (nMessages) {
        outString[totalBytesW++] = '\n';

        for (size_t i = 0; i < nMessages; i++) {
            const size_t size = strlen(messages[i]);
            memcpy(outString + totalBytesW, messages[i], size);
            totalBytesW += size;
            if (nMessages > 1) {
                outString[totalBytesW++] = '\n';
            }
        }
    }

    rString->length += totalBytesW - rString->length;

    return rString->length;
}

int32_t
client_loop(struct Client *client)
{
    struct RenderString renderString = {};
    init_render_string(&renderString);
    struct State gameState = {
        .client = client,
        .string = &renderString,
        .columns = client->gameSettings->field->columns,
        .field = client->gameSettings->field,
    };

    char *errorBuffer = malloc(128);

    clear_terminal();
    size_t bytes = client_render(gameState.client, gameState.string,
                                 (const char *[1]){labelMessages[6]}, 1);
    write(STDOUT_FILENO, gameState.string->renderData, bytes);

    while (1) {
        struct Payload data = {};
        int32_t mType = queue_recive(client->queueId, &data, -MSG_GAME_START);

        int64_t bytes = 0;
        _Bool needRender = false;
        switch (mType) {
        case MSG_ERROR:
            clear_terminal();
            bytes = process_error(&data, &gameState, errorBuffer);
            break;
        case MSG_GAME_END:
            bytes = process_game_end(&data, &gameState);
            break;
        case MSG_GAME_START:
            clear_terminal();
            bytes = process_turn_start(&data, &gameState);
            needRender = true;
            break;
        case MSG_TURN_START:
            clear_terminal();
            bytes = process_turn_start(&data, &gameState);
            needRender = true;
            break;
        case MSG_SERVER_ACK:
            clear_terminal();
            bytes = process_turn_end(&data, &gameState);
            needRender = true;
            break;
        }

        if (bytes == -9 || bytes == -6) {
            game_reset(client->gameSettings);
            memset(gameState.string->renderData, 0, gameState.string->size);
            continue;
        }

        if (bytes < 0) {
            free(gameState.string->renderData);
            free(errorBuffer);
            return bytes;
        }

        if (needRender) {
            clear_terminal();
            write(STDOUT_FILENO, gameState.string->renderData, bytes);
        }
    }
}

int32_t
init_client(struct Client *client, const struct ClientArgs *args)
{
    strcpy(client->playerName, args->playerName);
    client->isBot = args->isBot;
    client->pid = getpid();

    struct Logger *logger = get_logger();
    if (log_init(logger, STDOUT_FILENO, STDERR_FILENO) < 0) {
        LOG_ERROR("Errore inizializzazione log", "")
        return -1;
    }

    client_init_signals();

    return 1;
}

int32_t
parse_client_args(size_t argc, char const **argv, struct ClientArgs *args)
{
    if (argc != 3) {
        LOG_ERROR("Errore argomenti, utilizzo:\nF4Client {connectionQueue} "
                  "{playerName}",
                  "");
        return -1;
    }

    args->connQueueId = atoi(argv[1]);
    args->playerName = argv[2];
    args->isBot = false; // todo

    return 1;
}

int32_t
down_client(struct Client *client)
{
    game_destruct(client->gameSettings);
    _exit(EXIT_SUCCESS);
}

static const char *
get_bool_lab(_Bool var)
{
    return var ? "true" : "false";
}

void
print_client(struct Client *client)
{
    const char *const lab = "\n+--------------------------------------+\n";
    write(STDOUT_FILENO, lab, strlen(lab));

    LOG_INFO("Client", "");
    puts("----------------------------------------");
    printf("\tPid: %d\n\tPlayerName: %s"
           "\n\tQueueId: %d\n\tisBot: "
           "%s\n\tDisconnectionSignal: %d\n\tSymbol: %c\n\tOpponentSymbol: %c",
           client->pid, client->playerName, client->queueId,
           get_bool_lab(client->isBot), client->signalDisconnection,
           client->symbol, client->opponentSymbol);

    puts("+--------------------------------------+\n");
    fflush(stdout);
}