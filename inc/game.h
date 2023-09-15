#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct GameSettings {
    uint64_t movesCounter;
    struct GameField *field;
};

struct GameField {
    uint8_t rows;
    uint8_t columns;
    uint8_t **matrix;
    uint32_t
        *rowsIndex; // array con indice riga corrente occupata della colonna
};
/*
 * Inizializza il campo da gioco
 */
int32_t game_init(struct GameSettings *game, size_t rows, size_t columns);

_Bool game_check_win(struct GameField *field, char symbol);

int32_t game_set_point(struct GameField *field, size_t columnIndex,
                       char symbol);

int32_t game_destruct(struct GameSettings *game);

int32_t game_reset(struct GameSettings *game);

void game_set_point_index(struct GameField *field, uint8_t row, uint8_t column,
                          char symbol);

void print_game_field(struct GameSettings *game);

#endif