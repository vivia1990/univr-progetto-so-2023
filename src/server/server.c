#include "server.h"
#include "connection_manager.h"
#include "game.h"
#include "log.h"
#include "messages.h"
#include "queue_api.h"
#include "utils.h"
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

int32_t
init_server(struct Server *server, struct GameSettings *gameSettings,
            struct ServerArgs *args)
{
    server->pid = getpid();
    server_init_signals();

    struct Logger *logger = get_logger();
    if (log_init(logger, STDOUT_FILENO, STDERR_FILENO) < 0) {
        LOG_ERROR("Errore inizializzazione log", "")
        return -1;
    }

    server->logger = logger;
    game_init(gameSettings, args->rows, args->columns);
    server->gameSettings = gameSettings;

    return 1;
}

struct GameState {
    struct Client *currentPlayer;
    size_t currentPlayerIndex;
};

static int32_t
update_state(struct Server *server, struct GameState *state,
             const struct GameState *newState)
{
    server->currentPlayer = newState->currentPlayerIndex;
    state->currentPlayer = newState->currentPlayer;
    state->currentPlayerIndex = newState->currentPlayerIndex;

    return 1;
}

static struct Client *
get_connected_player(struct Server *server)
{
    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        if (!client->disconnected) {
            return client;
        }
    }

    return NULL;
}

static void
wait_queue_empty(const struct GameState *state, struct Server *server)
{
    const int32_t currQid = state->currentPlayer->queueId;
    const int32_t otherQid =
        server->players[!state->currentPlayerIndex]->queueId;
    size_t counter = 0;
    while ((queue_is_empty(currQid) == false ||
            queue_is_empty(otherQid) == false) &&
           (counter++ < 10)) {
        sleep(1);
    }
}

int32_t
server_loop(struct Server *server)
{
    LOG_INFO("Server game loop started, pid: %d", getpid())

    struct GameField *gameField = server->gameSettings->field;
    const uint32_t maxMoves = gameField->rows * gameField->columns;

    const struct GameState state = {};

    update_state(server, (struct GameState *)&state,
                 &(struct GameState){
                     .currentPlayer = server->players[0],
                     .currentPlayerIndex = 0,
                 });
    struct ServerGameResponse conn = {.updateField = false};
    queue_send_game(state.currentPlayer->queueId, &conn, sizeof conn,
                    MSG_GAME_START);

    while (1) {
        server->wasCtrlCPressed = false;

        struct ClientGameRequest req = {};
        LOG_INFO("Server receiving", "")
        int32_t mtype = queue_recive_game(state.currentPlayer->queueId, &req,
                                          sizeof req, MSG_CLIENT_MOVE);
        LOG_INFO("Received: %d", mtype)
        if (server->disconnectionHappened) { // todo refactor si può usare
                                             // direttamente il counter?
            LOG_INFO("Client Disconnesso", "")
            struct Client *client = get_connected_player(server);
            if (!client) {
                LOG_INFO("giocatori disconnesi", "")
                return -1;
            }
            LOG_INFO("vittoria player: %s", client->playerName)

            struct ErrorMsg error = {.errorCode = 601,
                                     .errorMsg =
                                         "Giocatore avversario disconnesso"};
            queue_send_error(client->queueId, &error);

            if (server->disconnectionCounter == 2) {
                LOG_INFO("giocatori disconnesi", "")
                return -1;
            }

            wait_queue_empty(&state, server);

            return -2;
        }

        if (server->timeoutHappened) {
            server->timeoutHappened = false;
            if (++state.currentPlayer->timeoutCounter > MAX_TIMEOUT_MATCH) {
                struct ErrorMsg lost = {
                    .errorCode = 600, .errorMsg = "Match perso per inattività"};
                queue_send_error(state.currentPlayer->queueId, &lost);

                struct ServerGameResponse resp = {
                    .endGame = true,
                    .winner = true,
                    .draw = false,
                    .column = 0,
                    .row = 0,
                    .updateField = false,
                };
                update_state(
                    server, (struct GameState *)&state,
                    &(struct GameState){
                        .currentPlayer =
                            server->players[!state.currentPlayerIndex],
                        .currentPlayerIndex = !state.currentPlayerIndex,
                    });

                queue_send_game(state.currentPlayer->queueId, &resp,
                                sizeof resp, MSG_GAME_END);
                kill(state.currentPlayer->pid, SIGALRM);
                kill(server->players[!state.currentPlayerIndex]->pid, SIGALRM);
                wait_queue_empty(&state, server);

                game_reset(server->gameSettings);

                return -3;
            }
            struct ErrorMsg error = {.errorCode = 408,
                                     .errorMsg =
                                         "Turno perso, ci hai messo troppo"};

            kill(state.currentPlayer->pid, SIGALRM);

            queue_send_error(state.currentPlayer->queueId, &error);

            update_state(
                server, (struct GameState *)&state,
                &(struct GameState){
                    .currentPlayer = server->players[!state.currentPlayerIndex],
                    .currentPlayerIndex = !state.currentPlayerIndex,
                });

            struct ServerGameResponse resp = {.updateField = false};
            queue_send_game(state.currentPlayer->queueId, &resp, sizeof resp,
                            MSG_GAME_START);

            continue;
        }

        const int32_t rowIndex =
            game_set_point(gameField, req.move, state.currentPlayer->symbol);
        if (rowIndex < 0) {

            struct ErrorMsg error = {.errorCode = 400,
                                     .errorMsg = "Mossa invalida"};
            queue_send_error(state.currentPlayer->queueId, &error);
            continue;
        }

        print_game_field(server->gameSettings);

        if (game_check_win(gameField, state.currentPlayer->symbol)) {
            // handle win
        }

        struct ServerGameResponse resp = {
            .endGame = false,
            .draw = false,
            .winner = false,
            .column = req.move,
            .row = rowIndex,
            .updateField = true,
        };

        // gioco continua, fine turno per il player
        queue_send_game(state.currentPlayer->queueId, &resp, sizeof resp,
                        MSG_SERVER_ACK); // @todo maybe turn_end?

        if (++server->gameSettings->movesCounter == maxMoves) {
            LOG_INFO("pareggio, mosse: %u", maxMoves)
            print_game_field(server->gameSettings);

            struct ServerGameResponse drawEnd = {0};
            drawEnd.draw = true;
            drawEnd.endGame = true;
            drawEnd.column = req.move;
            drawEnd.row = rowIndex;
            const int32_t currQid = state.currentPlayer->queueId;
            const int32_t otherQid =
                server->players[!state.currentPlayerIndex]->queueId;
            drawEnd.updateField = false;
            queue_send_game(currQid, &drawEnd, sizeof drawEnd, MSG_GAME_END);

            drawEnd.updateField = true; // ha già settatto il campo
            queue_send_game(otherQid, &drawEnd, sizeof drawEnd, MSG_GAME_END);

            wait_queue_empty(&state, server);

            return -4;
        }

        LOG_INFO("reset timeout player %s", state.currentPlayer->playerName)
        state.currentPlayer->timeoutCounter = 0;
        update_state(
            server, (struct GameState *)&state,
            &(struct GameState){
                .currentPlayer = server->players[!state.currentPlayerIndex],
                .currentPlayerIndex = !state.currentPlayerIndex,
            });

        // notifico lo start all'altro giocatore e torno in attesa
        queue_send_game(state.currentPlayer->queueId, &resp, sizeof resp,
                        MSG_TURN_START);
    }

    return 1;
}

struct Client *
create_client(struct ClientConnectionRequest *request, char symbol)
{

    struct Client *client = malloc(sizeof(struct Client));
    client->pid = request->clientPid;
    strcpy(client->playerName, request->playerName);
    client->timeoutCounter = 0;
    client->winner = false;
    client->symbol = symbol;

    ssize_t qId = create_queue();
    if (qId == -1) {
        return NULL;
    }
    client->queueId = qId;

    return client;
}

int32_t
down_server(struct Server *server)
{
    if (server->connMng) {
        if (conn_remove_manager(server->connMng) < 0) {
            LOG_ERROR("Errore rimozione manager", "")
        }
    }

    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        if (!client) {
            continue;
        }
        if (remove_queue(client->queueId) < 0) {
            LOG_ERROR("Impossibile rimuovere msgqueue: %d, client %s",
                      client->queueId, client->playerName);
        }

        free(client);
    }

    game_destruct(server->gameSettings);

    return 0;
}

int32_t
add_clients(struct Server *server, struct ServerArgs *args)
{
    server->playerCounter = 0;
    struct Client client = {};
    const size_t size = sizeof client;
    while (server->playerCounter < 2) {
        ssize_t bt = read(server->connMng->connServicePipe[0], &client, size);
        if (bt < 0) {
            LOG_ERROR("Errore inizializzazione client, counter %ld",
                      server->playerCounter);
            continue;
        }

        assert(server->playerCounter < 2);
        client.symbol = args->symbols[server->playerCounter];
        server->players[server->playerCounter] =
            memcpy(malloc(size), &client, size);

        server->playerCounter++;
    }

    return server->playerCounter;
}

static const char *
get_bool_lab(_Bool var)
{
    return var ? "true" : "false";
}

void
print_server(struct Server *server)
{
    const char *const lab = "\n+--------------------------------------+\n";
    write(STDOUT_FILENO, lab, strlen(lab));

    LOG_INFO("Server", "");
    puts("----------------------------------------");
    struct ConnectionManager *connMng = server->connMng;
    printf("\tPid: %d\n\tDisconnectionHappened: "
           "%d\n\tDisconnectionCounter: %d\n\tWasCTRLPressed: "
           "%s\n\tPlayerCounter: %d\n",
           server->pid, server->disconnectionHappened,
           server->disconnectionCounter, get_bool_lab(server->wasCtrlCPressed),
           server->playerCounter);

    if (connMng) {
        printf("\tConnectionServicePid: %d\n\tConnectionQueueId: "
               "%d\n\tInGame: "
               "%s\n\tIsListeningForConnection: %s\n",
               connMng->connServicePid, connMng->connQueueId,
               get_bool_lab(connMng->inGame),
               get_bool_lab(connMng->isListneningForConnection));
    }

    puts("\nClients:");
    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        printf("\tPid: %d\n\tPlayerName: %s\n\tQueueId: "
               "%d\n\tTimeoutCounter:  %d\n\twinner %d\n\tsymbol %c\n\n",
               client->pid, client->playerName, client->queueId,
               client->timeoutCounter, client->winner, client->symbol);
    }
    puts("+--------------------------------------+\n");
    fflush(stdout);
}