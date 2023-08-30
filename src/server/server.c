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
    server->inGame = false;
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

int32_t
server_loop(struct Server *server)
{
    LOG_INFO("Server game loop started, pid: %d", getpid())

    struct GameField *gameField = server->gameSettings->field;
    const uint32_t maxMoves = gameField->rows * gameField->columns;

    while (1) {
        const struct GameState state = {};

        update_state(server, (struct GameState *)&state,
                     &(struct GameState){
                         .currentPlayer = server->players[0],
                         .currentPlayerIndex = 0,
                     });
        struct ServerGameResponse conn = {};
        queue_send_game(state.currentPlayer->queueId, &conn, sizeof conn,
                        MSG_GAME_START);

        while (1) {
            struct ClientGameRequest req = {};
            queue_recive_game(state.currentPlayer->queueId, &req, sizeof req,
                              MSG_CLIENT_MOVE);

            int32_t rowIndex = game_set_point(gameField, req.move,
                                              state.currentPlayer->symbol);
            if (rowIndex < 0) {

                struct ErrorMsg error = {.errorCode = 400,
                                         .errorMsg = "Mossa invalida"};
                queue_send_error(state.currentPlayer->queueId, &error,
                                 sizeof error);
                continue;
            }

            print_game_field(server->gameSettings);

            if (++server->gameSettings->movesCounter == maxMoves) {
                LOG_INFO("pareggio, mosse: %u", maxMoves)
            }

            if (game_check_win(gameField, state.currentPlayer)) {
                // handle win
            }

            struct ServerGameResponse resp = {
                .endGame = false,
                .draw = false,
                .winner = false,
                .column = req.move,
                .row = rowIndex,
            };
            // gioco continua, fine turno per il player
            queue_send_game(state.currentPlayer->queueId, &resp, sizeof resp,
                            MSG_SERVER_ACK); // @todo maybe turn_end?

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
    }

    return 1;
}

struct Client *
create_client(struct ClientConnectionRequest *request)
{

    struct Client *client = malloc(sizeof(struct Client));
    client->pid = request->clientPid;
    strcpy(client->playerName, request->playerName);
    client->timeoutCounter = 0;
    client->winner = false;

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
    if (conn_remove_manager(server) < 0) {
        LOG_ERROR("Errore rimozione manager", "")
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

    close(server->connServicePipe[0]);
    close(server->connServicePipe[1]);

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
        ssize_t bt = read(server->connServicePipe[0], &client, size);
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

    return 1;
}

void
print_server(struct Server *server)
{
    const char *const lab = "\n+--------------------------------------+\n";
    write(STDOUT_FILENO, lab, strlen(lab));
    LOG_INFO("Server", "");
    puts("----------------------------------------");
    printf("\tPid: %d\n\tConnectionServicePid: %d\n\tConnectionQueueId: "
           "%d\n\tInGame: "
           "%d\n\tIsListeningForConnection %d\n",
           server->pid, server->connServicePid, server->connQueueId,
           server->inGame, server->isListneningForConnection);

    puts("\nClients:");
    for (size_t i = 0; i < server->playerCounter; i++) {
        struct Client *client = server->players[i];
        printf("\tPid: %d\n\tPlayerName: %s\n\tQueueId: "
               "%d\n\tTimeoutCounter:  %d\n\twinner %d\n\n\tsymbol %c\n\n",
               client->pid, client->playerName, client->queueId,
               client->timeoutCounter, client->winner, client->symbol);
    }
    puts("+--------------------------------------+\n");
    fflush(stdout);
}

void
sig_int_handler()
{
    struct Server *server = get_server();
    if (!server->wasCtrlCPressed) {
        server->wasCtrlCPressed = true;
    }
    else {
        LOG_INFO("CTRL-C premuto ripetutamente, terminazione Server", "")
        down_server(server);
        exit(EXIT_SUCCESS);
    }
}

int32_t
server_init_signals()
{
    sigset_t signals;
    sigfillset(&signals);

    sigdelset(&signals, SIGINT);
    sigprocmask(SIG_SETMASK, &signals, NULL);

    signal(SIGINT, sig_int_handler);

    return 1;
}