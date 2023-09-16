#include "client.h"
#include "game.h"
#include "log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t
client_parse_args(int argc, char *argv[], struct ClientArgs *args)
{
    if (argc != 3) {
        printf("Utilizzo: %s <Server connection Queue> <playerName>\n",
               argv[0]);
        return -1;
    }

    args->connQueueId = atoi(argv[1]);

    if (strlen(argv[3]) > 64) {
        printf("Il nome del giocatore deve essere di 64 caratteri massimo\n");
        return -1;
    }

    args->playerName = argv[3];

    return 0;
}

int
main(int argc, char const *argv[])
{

    struct ClientArgs args = {};
    if (parse_client_args(argc, argv, &args) < 0) {
        return EXIT_FAILURE;
    }

    struct Client *client = get_client();
    init_client(client, &args);

    struct GameField field = {0};
    struct GameSettings settings = {0};
    settings.field = &field;
    client->gameSettings = &settings;

    if (connect_to_server(&args, client) < 0) {
        LOG_INFO("Errore connessione server", "");
        return EXIT_FAILURE;
    }
    game_init(client->gameSettings, client->gameSettings->field->rows,
              client->gameSettings->field->columns);

    client_loop(client);

    game_destruct(client->gameSettings);

    return EXIT_SUCCESS;
}

struct Client *
get_client()
{
    static struct Client client = {0};
    return &client;
}