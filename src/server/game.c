#include "game.h"
#include "log.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t
game_init(struct GameSettings *game, size_t rows, size_t columns)
{
    game->movesCounter = 0;

    struct GameField *field = game->field;
    field->columns = columns;
    field->rows = rows;
    field->matrix = malloc(rows * sizeof(uint8_t *));
    assert(field->matrix != NULL);

    for (size_t i = 0; i < rows; i++) {
        field->matrix[i] = malloc(columns * sizeof(uint8_t));
        assert(field->matrix[i] != NULL);
        memset(field->matrix[i], 0x20, columns);
    }

    // un indice per colonna
    field->rowsIndex = malloc(sizeof(uint32_t) * columns);
    assert(field->rowsIndex != NULL);

    for (size_t i = 0; i < columns; i++) {
        field->rowsIndex[i] = rows;
    }

    return 1;
}

int32_t
game_reset(struct GameSettings *game)
{
    struct GameField *field = game->field;

    uint32_t columns = field->columns;
    uint32_t rows = field->rows;
    for (size_t i = 0; i < columns; i++) {
        field->rowsIndex[i] = rows;
    }

    for (size_t i = 0; i < rows; i++) {
        memset(field->matrix[i], 0x20, columns);
    }

    game->movesCounter = 0;

    return 1;
}

int32_t
game_destruct(struct GameSettings *game)
{
    LOG_INFO("Eliminazione Campo di gioco", "")

    struct GameField *field = game->field;
    for (size_t i = 0; i < field->rows; i++) {
        free(field->matrix[i]);
    }

    free(field->matrix);
    free(field->rowsIndex);

    return 1;
}

void
print_game_field(struct GameSettings *game)
{

    LOG_INFO("Matrice di gioco\n", "")
    const uint32_t offsetStart = 2;

    printf("%*c", offsetStart, 0x20);
    for (uint32_t i = 0; i < game->field->columns; i++) {
        printf("%*d%*c", 3, i, -1, 0x20);
    }
    puts("\n");

    for (size_t i = 0; i < game->field->rows; i++) {
        printf("%*ld", 2, i);
        for (size_t j = 0; j < game->field->columns; j++) {
            printf("%*c%*c", 3, game->field->matrix[i][j], -1, 0x20);
        }
        putc('\n', stdout);
    }
    putc('\n', stdout);
    fflush(stdout);
}

int32_t
game_set_point(struct GameField *field, size_t columnIndex, char symbol)
{
    if (columnIndex > field->columns - 1) {
        LOG_ERROR("Errore indice colonna fuori range %ld", columnIndex)
        return -1;
    }

    const uint32_t currentRow = field->rowsIndex[columnIndex];
    if (currentRow == 0) {
        return -1;
    }

    field->matrix[currentRow - 1][columnIndex] = symbol;

    return field->rowsIndex[columnIndex] = currentRow - 1;
}

_Bool
game_check_win(struct GameField *field, char symbol)
{
    return false;
}