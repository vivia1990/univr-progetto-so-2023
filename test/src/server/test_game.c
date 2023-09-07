#include "assert.h"
#include "game.h"
#include "test.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t
test_init_game()
{
    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 7);

    struct GameField *field = game.field;
    assert(field->columns == 7 && field->rows == 5);

    for (size_t i = 0; i < field->rows; i++) {
        for (size_t j = 0; j < field->rows; j++) {
            assert(field->matrix[i][j] == 0x20);
        }
    }

    for (size_t i = 0; i < field->columns; i++) {
        assert(field->rowsIndex[i] == field->rows);
    }

    game_destruct(&game);

    TEST_PASSED("test_init_game", "")

    return 1;
}

int32_t
test_game_field()
{
    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 7);

    struct GameField *field = game.field;
    assert(field->columns == 7 && field->rows == 5);

    game_destruct(&game);

    TEST_PASSED("test_game_field", "")

    return 1;
}

int32_t
test_game_set_point_1()
{
    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 5);

    struct GameField *field = game.field;
    assert(field->columns == 5 && field->rows == 5);

    assert(game_set_point(field, 5, 'X') == -1);
    assert(game_set_point(field, 6, 'X') == -1);

    game_destruct(&game);

    TEST_PASSED("test_game_set_point_1", "")

    return 1;
}

static inline int32_t
set_and_test(struct GameField *field, size_t columnIndex, char symbol)
{
    const int32_t ret = game_set_point(field, columnIndex, symbol);
    if (ret == -1) {
        return -1;
    }
    assert(field->matrix[field->rowsIndex[columnIndex]][columnIndex] == symbol);

    return ret;
}

int32_t
test_game_set_point_2()
{
    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 5);

    struct GameField *field = game.field;
    assert(field->columns == 5 && field->rows == 5);

    assert(set_and_test(field, 0, 'X') == field->rows - 1);
    assert(set_and_test(field, 0, 'X') == field->rows - 2);
    assert(set_and_test(field, 0, 'X') == field->rows - 3);
    assert(set_and_test(field, 0, 'X') == field->rows - 4);
    assert(set_and_test(field, 0, 'X') == field->rows - 5);
    assert(set_and_test(field, 0, 'X') == -1);

    set_and_test(field, 1, 'X');
    set_and_test(field, 2, 'X');
    set_and_test(field, 3, 'X');
    set_and_test(field, 4, 'X');

    assert(field->rowsIndex[1] == field->rows - 1);
    assert(field->rowsIndex[2] == field->rows - 1);
    assert(field->rowsIndex[3] == field->rows - 1);
    assert(field->rowsIndex[4] == field->rows - 1);

    print_game_field(&game);

    game_destruct(&game);

    TEST_PASSED("test_game_set_point_2", "")

    return 1;
}

int32_t
test_game_set_point_3()
{
    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 5);

    struct GameField *field = game.field;
    assert(field->columns == 5 && field->rows == 5);

    assert(set_and_test(field, 0, 'X') == field->rows - 1);
    assert(set_and_test(field, 0, 'X') == field->rows - 2);
    assert(set_and_test(field, 0, 'O') == field->rows - 3);
    assert(set_and_test(field, 0, 'O') == field->rows - 4);

    assert(set_and_test(field, 1, 'X') == field->rows - 1);
    assert(set_and_test(field, 2, 'O') == field->rows - 1);
    assert(set_and_test(field, 3, 'X') == field->rows - 1);
    assert(set_and_test(field, 4, 'O') == field->rows - 1);

    print_game_field(&game);

    game_destruct(&game);

    TEST_PASSED("test_game_set_point_3", "")

    return 1;
}

int32_t
test_print_game_field()
{

    struct GameField gf = {};
    struct GameSettings game = {.field = &gf};

    game_init(&game, 5, 5);

    for (size_t i = 0; i < game.field->rows; i++) {
        for (size_t j = 0; j < game.field->columns; j++) {
            game.field->matrix[i][j] = 'X';
        }
    }

    print_game_field(&game);

    game_destruct(&game);

    return 1;
};

int
main(int argc, char const *argv[])
{
    test_init_game();
    test_game_field();
    test_game_set_point_1();
    test_game_set_point_2();
    test_game_set_point_3();
    test_print_game_field();

    return EXIT_SUCCESS;
}