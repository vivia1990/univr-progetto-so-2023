#include "game.h"
#include "log.h"
#include "server.h"
#include "test.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
test_game_win_1()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 5, 7);

    settings.field->matrix[0][0] = 'O';
    settings.field->matrix[1][1] = 'X';
    settings.field->matrix[2][2] = 'O';
    settings.field->matrix[3][3] = 'O';
    settings.field->matrix[4][4] = 'X';

    uint8_t player = 'O';
    _Bool winner = game_check_win(settings.field, player, kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
test_game_win_2()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 5, 7);

    settings.field->matrix[4][0] = 'O';
    settings.field->matrix[4][1] = 'O';
    settings.field->matrix[4][2] = 'O';
    settings.field->matrix[4][3] = 'O';

    uint8_t player = 'O';
    _Bool winner = game_check_win(settings.field, player, kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
test_game_win_3()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 5, 5);

    settings.field->matrix[0][0] = 'X';
    settings.field->matrix[1][0] = 'X';
    settings.field->matrix[2][0] = 'X';
    settings.field->matrix[3][0] = 'X';
    settings.field->matrix[4][0] = 'O';
    print_game_field(&settings);
    uint8_t player = 'X';
    _Bool winner = game_check_win(settings.field, player, kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
test_game_win_4()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 4, 7);
    uint8_t board[4][7] = {
        {' ', ' ', 'O', 'O', 'X', ' ', 'X'},
        {' ', ' ', 'X', 'O', 'X', ' ', 'O'},
        {' ', 'X', 'O', 'O', 'O', ' ', 'X'},
        {'X', 'O', 'X', 'X', 'X', 'O', 'O'},
    };

    for (size_t i = 0; i < settings.field->rows; i++) {
        for (size_t j = 0; j < settings.field->columns; j++) {
            settings.field->matrix[i][j] = board[i][j];
        }
    }

    print_game_field(&settings);
    uint8_t player = 'O';
    _Bool winner = game_check_win(settings.field, player, kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
test_game_win_5()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 4, 7);
    uint8_t board[4][7] = {
        {' ', ' ', 'O', 'X', 'X', ' ', 'X'},
        {' ', ' ', 'X', 'O', 'X', ' ', 'O'},
        {' ', 'X', 'O', 'O', 'O', ' ', 'X'},
        {'X', 'O', 'X', 'X', 'O', 'X', 'O'},
    };

    for (size_t i = 0; i < settings.field->rows; i++) {
        for (size_t j = 0; j < settings.field->columns; j++) {
            settings.field->matrix[i][j] = board[i][j];
        }
    }

    print_game_field(&settings);
    uint8_t player = 'X';
    _Bool winner = game_check_win(settings.field, player, kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
test_game_win_6()
{
    LOG_INFO("TEST %s", __func__);
    struct GameField field = {};
    struct GameSettings settings = {.field = &field};

    uint8_t *const *kernels[kernels_length] = {
        [horizontal] = NULL,
        [vertical] = NULL,
        [diagonal_l] = NULL,
        [diagonal_r] = NULL,
    };

    game_init_kernels(kernels);
    game_init(&settings, 4, 7);
    uint8_t board[4][7] = {
        {' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'X', ' ', ' ', ' ', ' '},
        {'O', 'X', 'O', ' ', ' ', ' ', ' '},
        {'X', 'O', 'O', ' ', ' ', 'X', ' '},
    };

    for (size_t i = 0; i < settings.field->rows; i++) {
        for (size_t j = 0; j < settings.field->columns; j++) {
            settings.field->matrix[i][j] = board[i][j];
        }
    }

    print_game_field(&settings);
    uint8_t player = 'X';
    _Bool winner = game_check_win(settings.field, player, kernels) ||
                   game_check_win(settings.field, 'O', kernels);
    game_check_win(settings.field, 'O', kernels);

    game_destruct(&settings);
    game_destruct_kernels((uint8_t ***)kernels);
    TEST_PASSED("%s", __func__)

    return winner;
}

int
main(int argc, char const *argv[])
{
    assert(test_game_win_1() == false);
    assert(test_game_win_2() == true);
    assert(test_game_win_3() == true);
    assert(test_game_win_4() == true);
    assert(test_game_win_5() == true);
    assert(test_game_win_6() == false);
    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}
