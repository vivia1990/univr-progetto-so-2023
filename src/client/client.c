
#include "client.h"
#include "game.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "utils.h"

#include <errno.h>
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
    uint8_t columns;
};
static int32_t process_error(struct Payload *pl, struct State *state,
                             char *buffer);
static int32_t process_game_end(struct Payload *pl, struct State *state);
static int32_t process_turn_start(struct Payload *pl, struct State *state);
static int32_t process_turn_end(struct Payload *pl, struct State *state);

extern int32_t queue_recive(int32_t qId, struct Payload *pl, int32_t mType);

const char *const messages[] = {
    "==> Congratulazioni, hai vinto!",
    "==> Sconfitta :(",
    "==> Gioco terminato, Pareggio\n",
    "==> Mossa precedente non valida!\n\n",
    "==> Attesa mossa altro giocatore...",
};

int32_t
connect_to_server(struct ClientArgs *args, struct Client *client)
{
    struct ClientConnectionRequest req = {.clientPid = getpid(),
                                          .typeResp = getpid()};

    if (queue_send_connection(args->connQueueId, &req, sizeof req,
                              MSG_CONNECTION) < 0) {
        LOG_ERROR("Errore inivio msg per connessione, qId: %d",
                  args->connQueueId)
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
    const char *const msgPrefix = "==> ";

    const size_t size = strlen(msgPrefix);
    size_t length = size + strlen(errorMsg.errorMsg);
    memcpy(buffer, msgPrefix, size);
    strcpy(buffer + size, errorMsg.errorMsg);
    buffer[length++] = '\n';
    buffer[length++] = '\n';

    write(STDOUT_FILENO, buffer, length);

    return process_turn_start(pl, state);
}

static int32_t
process_game_end(struct Payload *pl, struct State *state)
{
    struct ServerGameResponse resp = {};
    memcpy(&resp, pl->payload, sizeof resp);
    game_set_point_index(state->field, resp.row, resp.column,
                         state->client->opponentSymbol);
    if (resp.winner) {
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){messages[0]}, 1);
        // todo ask rematch

        struct ClientGameRequest req = {.move = 0, .restartMatch = false};
        queue_send_game(state->client->queueId, &resp, sizeof resp,
                        MSG_CLIENT_MOVE); // todo nuovo tipo?

        return bytes;
    }

    if (resp.draw) {
        LOG_INFO("pareggio", "");
        size_t bytes = client_render(state->client, state->string,
                                     (const char *[1]){messages[2]}, 1);
        write(STDOUT_FILENO, state->string->renderData, bytes);

        return -4;
    }

    if (resp.endGame) {
        return client_render(state->client, state->string,
                             (const char *[1]){messages[1]}, 1);
    }
    return 1;
}

int32_t
client_get_move(int32_t maxColumns, struct Client *client)
{
    char number[4] = {0};

    int32_t move = 0;
    while (1) {
        errno = 0;
        read(STDIN_FILENO, number, 3);
        if (client->timeoutHappened) {
            LOG_ERROR("Ci hai impiegato troppo tempo", "");
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
            write(STDOUT_FILENO, messages[3], strlen(messages[3]));
            write(STDOUT_FILENO, state->string->renderData, bytes);
            continue;
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
                         (const char *[1]){messages[4]}, 1);
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

    for (uint8_t i = 0; i < field->columns; i++) {
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

        uint32_t totBytesMsg = 0;
        for (size_t i = 0; i < nMessages; i++) {
            const size_t size = strlen(messages[i]);
            memcpy(outString + totalBytesW, messages[i], size);
            totBytesMsg += size;
        }

        totalBytesW += totBytesMsg;
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

    print_client(client);

    char *errorBuffer = malloc(128);

    while (1) {
        struct Payload data = {};
        int32_t mType = queue_recive(client->queueId, &data, -MSG_GAME_START);

        LOG_INFO("Received %d", mType)

        size_t bytes = 0;
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

        if (bytes == -4) { // pareggio
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