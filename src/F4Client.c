#include "client.h"
#include "game.h"
#include "log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char const *argv[])
{

    struct ClientArgs args = {};
    parse_client_args(argc, argv, &args);

    struct Client *client = get_client();
    init_client(client, &args);

    struct GameField field = {0};
    struct GameSettings settings = {0};
    settings.field = &field;
    client->gameSettings = &settings;

    connect_to_server(&args, client);
    LOG_INFO("In attesa di altri giocatori", "")
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