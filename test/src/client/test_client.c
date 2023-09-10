#include "client.h"
#include "game.h"
#include "log.h"
#include "test.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void
clearLine()
{
    printf("\033[H\033[J");
}

int32_t
test_client_render()
{
    LOG_INFO("test: %s", __FUNCTION__);
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
    struct RenderString rString = {};
    init_render_string(&rString);
    field.matrix[field.rows - 1][4] = 'X';
    field.matrix[field.rows - 1][3] = 'O';
    clearLine();
    size_t bytetW = client_render(client, &rString);
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);

    field.matrix[field.rows - 1][2] = 'X';
    field.matrix[field.rows - 1][1] = 'O';
    bytetW = client_render(client, &rString);
    sleep(4);
    clearLine();
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);

    field.matrix[field.rows - 1][0] = 'X';
    field.matrix[field.rows - 1][6] = 'O';
    bytetW = client_render(client, &rString);
    sleep(4);
    clearLine();
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);

    assert(strlen(rString.renderData) == bytetW);

    free(rString.renderData);

    game_destruct(client->gameSettings);

    TEST_PASSED("end: %s", __FUNCTION__);

    return EXIT_SUCCESS;
}

int
main(int argc, char const *argv[])
{
    test_client_render();
    return EXIT_SUCCESS;
}

struct Client *
get_client()
{
    static struct Client client = {0};
    return &client;
}