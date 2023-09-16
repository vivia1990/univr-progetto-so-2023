#include "client.h"
#include "game.h"
#include "log.h"
#include "test.h"
#include "utils.h"

#include <errno.h>
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
    client->opponentSymbol = 'O';
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
    clear_terminal();
    size_t bytetW = client_render(client, &rString, NULL, 0);
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);

    field.matrix[field.rows - 1][2] = 'X';
    field.matrix[field.rows - 1][1] = 'O';
    bytetW = client_render(client, &rString, NULL, 0);
    sleep(4);
    clear_terminal();
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);

    field.matrix[field.rows - 1][0] = 'X';
    field.matrix[field.rows - 1][6] = 'O';

    const char *const askMsg = "==> È il tuo turno\n==> Scegli una colonna: ";
    bytetW =
        client_render(client, &rString, (const char *[2]){askMsg, askMsg}, 2);
    sleep(4);
    clear_terminal();
    printf("bt_written: %ld, strlen: %ld\n\n%s", bytetW,
           strlen(rString.renderData), rString.renderData);
    fflush(stdout);
    assert(strlen(rString.renderData) == bytetW);

    free(rString.renderData);

    game_destruct(client->gameSettings);

    TEST_PASSED("end: %s", __FUNCTION__);

    return EXIT_SUCCESS;
}

int32_t
test_client_get_move()
{
    const char *const name = "mich";
    struct ClientArgs args = {
        .connQueueId = 1,
        .isBot = false,
        .playerName = name,
    };
    struct Client *client = get_client();

    const char *const msg =
        "        1     2     3     4     5     6     7\n     "
        "*-----+-----+-----+-----+-----+-----+-----*\n  1  |     |     |     | "
        "    |     |     |     |\n     "
        "|-----+-----+-----+-----+-----+-----+-----+\n  2  |     |     |     | "
        "    |     |     |     |\n     "
        "|-----+-----+-----+-----+-----+-----+-----+\n  3  |     |     |     | "
        "    |     |     |     |\n     "
        "|-----+-----+-----+-----+-----+-----+-----+\n  4  |     |     |     | "
        "    |     |     |     |\n     "
        "|-----+-----+-----+-----+-----+-----+-----+\n  5  |  X  |  O  |  X  | "
        " O  |  X  |     |  O  |\n     "
        "*-----+-----+-----+-----+-----+-----+-----*\n==> È il tuo turno\n==> "
        "Scegli una colonna: ";

    init_client(client, &args);
    const char *const errorMsg = "==> Mossa precedente non valida!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    int32_t move = 0; // rand() % state->columns;
    while (move <= 0) {
        move = client_get_move(7, client);
        if (move == -1) {
            clear_terminal();
            write(STDOUT_FILENO, errorMsg, strlen(errorMsg));
            write(STDOUT_FILENO, msg, strlen(msg));
            continue;
        }
    }

    return 1;
}

int
main(int argc, char const *argv[])
{
    test_client_render();
    // test_client_get_move();
    return EXIT_SUCCESS;
}

struct Client *
get_client()
{
    static struct Client client = {0};
    return &client;
}