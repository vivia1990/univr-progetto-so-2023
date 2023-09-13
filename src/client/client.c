
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

extern int32_t queue_recive(int32_t qId, struct Payload *pl, int32_t mType);

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
process_error(struct Payload *pl, struct State *state)
{
    struct ErrorMsg errorMsg = {};
    memcpy(&errorMsg, pl, sizeof errorMsg);

    LOG_ERROR("ErrorCode %d, message: %s", errorMsg.errorCode,
              errorMsg.errorMsg)
    strcpy(state->string->renderData, errorMsg.errorMsg);

    return 1;
}

static int32_t
process_game_end(struct Payload *pl, struct State *state)
{
    return 1;
}

int32_t
client_get_move(int32_t maxColumns, struct Client *client)
{
    const char *const askMsg =
        "==> È il tuo turno\n==> Scegli una colonna e premi invio: ";
    const size_t size = strlen(askMsg);

    write(STDOUT_FILENO, askMsg, size);
    char number[4] = {0};

    int32_t move = 0;
    while (1) {
        errno = 0;
        read(STDIN_FILENO, number, 3);
        if (client->timeoutHappened) {
            move = -1;
            LOG_ERROR("Ci hai impiegato troppo tempo", "");
            client->timeoutHappened = false;

            resume_file_block(STDIN_FILENO);
            break;
        }

        number[3] = '\0';

        move = atoi(number);
        if ((number[0] != '0' && move == 0) || move > maxColumns) {
            LOG_ERROR("Inserito un numero di colonna non valido %s. Riprovare",
                      number);
            write(STDOUT_FILENO, askMsg, size);
            memset(number, 0, 4);
            continue;
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

        state->field->matrix[resp.row][resp.column] =
            state->client->opponentSymbol;
    }
    const char *const askMsg = "==> È il tuo turno\n\tScegli una colonna: ";
    ssize_t bytes = client_render(state->client, state->string,
                                  (const char *[1]){askMsg}, 1);

    write(STDOUT_FILENO, state->string, bytes);

    int32_t move = client_get_move(state->columns, state->client);
    if (move == -1) {
        // todo timeout
    }

    struct ClientGameRequest req = {.move = move, .restartMatch = false};
    queue_send_game(state->client->queueId, &req, sizeof req, MSG_CLIENT_MOVE);

    return 1;
}

static int32_t
process_turn_end(struct Payload *pl, struct State *state)
{
    return 1;
}

int32_t
init_render_string(struct RenderString *obj)
{
    obj->renderData = calloc(1024, 1);
    obj->size = 1024;
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
    const char *const padStr = "-----+";
    const size_t sizePadStr = strlen(padStr);

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

    clear_terminal();
    while (1) {
        struct Payload data = {};
        int32_t mType = queue_recive(client->queueId, &data, -MSG_TURN_START);

        clear_terminal();
        switch (mType) {
        case MSG_ERROR:
            process_error(&data, &gameState);
            break;
        case MSG_GAME_END:
            process_game_end(&data, &gameState);
            break;
        case MSG_GAME_START:
            process_turn_start(&data, &gameState);
            break;
        case MSG_TURN_START:
            process_turn_start(&data, &gameState);
            break;
        case MSG_SERVER_ACK:
            process_turn_end(&data, &gameState);
            break;
        }

        client_render(client, gameState.string, NULL, 0);
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
        LOG_ERROR("Errore argomenti", "");
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
           "%s\n\tDisconnectionSignal: %d\n",
           client->pid, client->playerName, client->queueId,
           get_bool_lab(client->isBot), client->signalDisconnection);

    puts("+--------------------------------------+\n");
    fflush(stdout);
}