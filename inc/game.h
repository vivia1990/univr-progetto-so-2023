#ifndef GAME_H
#define GAME_H

typedef struct Client Client;

#include "server.h"
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
int32_t game_check_win(struct GameField *field, struct Client *player);
int32_t game_set_point(struct GameField *field, size_t column, char symbol);
int32_t game_destruct(struct GameSettings *game);
void print_game_field(struct GameSettings *game);

#endif