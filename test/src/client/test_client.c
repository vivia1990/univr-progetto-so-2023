#include "client.h"
#include "game.h"
#include "log.h"
#include "test.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t
test_init_client()
{
    const size_t argc = 3;
    const char *argv[] = {"test_init_client", "3", "gianni"};
    struct ClientArgs args = {};
    parse_client_args(argc, argv, &args);

    struct Client *client = get_client();
    init_client(client, &args);

    struct GameField field = {0};
    struct GameSettings settings = {0};
    settings.field = &field;
    client->gameSettings = &settings;

    client->symbol = 'X';
    client->queueId = 1;
    client->serverPid = 1;
    client->signalDisconnection = 10;
    client->gameSettings->field->columns = 7;
    client->gameSettings->field->rows = 5;

    game_init(client->gameSettings, settings.field->rows,
              settings.field->columns);

    print_game_field(client->gameSettings);

    print_client(client);

    game_destruct(client->gameSettings);

    return 1;
}

int
main(int argc, char const *argv[])
{
    test_init_client();
    return EXIT_SUCCESS;
}

struct Client *
get_client()
{
    static struct Client client = {0};
    return &client;
}