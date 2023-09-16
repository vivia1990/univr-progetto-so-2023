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

int32_t
game_init_kernels(uint8_t *const *kernels[kernels_length])
{
    const uint32_t indexMap[kernels_length] = {
        [horizontal] = horizontal,
        [vertical] = vertical,
        [diagonal_l] = diagonal_l,
        [diagonal_r] = diagonal_r,
    };

    const uint32_t kernelsArr[kernels_length][2] = {
        [horizontal] = {1, WIN_SEQUENCE_SIZE},
        [vertical] = {WIN_SEQUENCE_SIZE, 1},
        [diagonal_l] = {WIN_SEQUENCE_SIZE, WIN_SEQUENCE_SIZE},
        [diagonal_r] = {WIN_SEQUENCE_SIZE, WIN_SEQUENCE_SIZE},
    };

    for (size_t kernIndex = 0; kernIndex < kernels_length; kernIndex++) {
        uint8_t **kern = malloc(kernelsArr[kernIndex][0] * sizeof(uint8_t *));
        for (size_t i = 0; i < kernelsArr[kernIndex][0]; i++) {
            const size_t size = kernelsArr[kernIndex][1] * sizeof(uint8_t);
            kern[i] = malloc(size);
            memset(kern[i], 0x20, size);
        }

        for (size_t i = 0; i < kernelsArr[kernIndex][0]; i++) {
            for (size_t j = 0; j < kernelsArr[kernIndex][1]; j++) {
                printf("%c ", kern[i][j]);
            }
            printf("\n");
        }

        kernels[indexMap[kernIndex]] = kern;
    }

    return 1;
}

int32_t
game_destruct_kernels(uint8_t **kernels[kernels_length])
{
    const uint32_t kernelsArr[kernels_length][2] = {
        [horizontal] = {1, WIN_SEQUENCE_SIZE},
        [vertical] = {WIN_SEQUENCE_SIZE, 1},
        [diagonal_l] = {WIN_SEQUENCE_SIZE, WIN_SEQUENCE_SIZE},
        [diagonal_r] = {WIN_SEQUENCE_SIZE, WIN_SEQUENCE_SIZE},
    };

    for (size_t i = 0; i < kernels_length; i++) {
        for (size_t j = 0; j < kernelsArr[i][0]; j++) {
            free(kernels[i][j]);
        }

        free(kernels[i]);
    }

    return 1;
}

static void
init_horizontal(uint8_t horizontalKern[1][WIN_SEQUENCE_SIZE], uint8_t symbol)
{
    memset(horizontalKern[0], symbol, (sizeof symbol) * WIN_SEQUENCE_SIZE);
}

static void
init_vertical(uint8_t vertical[WIN_SEQUENCE_SIZE][1], uint8_t symbol)
{
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        vertical[i][0] = symbol;
    }
}

static void
init_diagonal_r(uint8_t diagonalKern[WIN_SEQUENCE_SIZE][WIN_SEQUENCE_SIZE],
                uint8_t symbol)
{
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        diagonalKern[i][i] = symbol;
    }
}

static void
init_diagonal_l(uint8_t diagonalKern[WIN_SEQUENCE_SIZE][WIN_SEQUENCE_SIZE],
                uint8_t symbol)
{
    size_t counter = WIN_SEQUENCE_SIZE - 1;
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        diagonalKern[i][counter--] = symbol;
    }
}

_Bool
game_check_win(struct GameField *field, uint8_t player,
               uint8_t *const *const kernels[kernels_length])
{
    uint8_t **board = field->matrix;
    uint32_t rows = field->rows;
    uint32_t cols = field->columns;

    uint8_t horizontalKern[1][WIN_SEQUENCE_SIZE];
    for (size_t i = 0; i < 1; i++) {
        for (size_t j = 0; j < WIN_SEQUENCE_SIZE; j++) {
            horizontalKern[i][j] = kernels[horizontal][i][j];
        }
    }
    init_horizontal(horizontalKern, player);

    uint8_t verticalKern[WIN_SEQUENCE_SIZE][1];
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        for (size_t j = 0; j < 1; j++) {
            verticalKern[i][j] = kernels[vertical][i][j];
        }
    }
    init_vertical(verticalKern, player);

    uint8_t diagonalKernR[WIN_SEQUENCE_SIZE][WIN_SEQUENCE_SIZE];
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        for (size_t j = 0; j < WIN_SEQUENCE_SIZE; j++) {
            diagonalKernR[i][j] = kernels[diagonal_r][i][j];
        }
    }
    init_diagonal_r(diagonalKernR, player);

    uint8_t diagonalKernL[WIN_SEQUENCE_SIZE][WIN_SEQUENCE_SIZE];
    for (size_t i = 0; i < WIN_SEQUENCE_SIZE; i++) {
        for (size_t j = 0; j < WIN_SEQUENCE_SIZE; j++) {
            diagonalKernL[i][j] = kernels[diagonal_l][i][j];
        }
    }
    init_diagonal_l(diagonalKernL, player);

    for (uint32_t i = 0; i < rows; i++) {
        for (uint32_t j = 0; j < cols; j++) {
            if (j <= cols - WIN_SEQUENCE_SIZE) {
                uint32_t sum = 0;
                for (uint32_t k = 0; k < WIN_SEQUENCE_SIZE; k++) {
                    if (board[i][j + k] == horizontalKern[0][k]) {
                        sum++;
                    }
                }
                if (sum == WIN_SEQUENCE_SIZE)
                    return true;
            }

            if (i <= rows - WIN_SEQUENCE_SIZE) {
                uint32_t sum = 0;
                for (uint32_t k = 0; k < WIN_SEQUENCE_SIZE; k++) {
                    if (board[i + k][j] == verticalKern[k][0]) {
                        sum++;
                    }
                }
                if (sum == WIN_SEQUENCE_SIZE)
                    return true;
            }

            if (i <= rows - WIN_SEQUENCE_SIZE &&
                j <= cols - WIN_SEQUENCE_SIZE) {
                uint32_t sum1 = 0, sum2 = 0;
                for (uint32_t k = 0; k < WIN_SEQUENCE_SIZE; k++) {
                    if (board[i + k][j + k] == diagonalKernR[k][k]) {
                        sum1++;
                    }
                    if (board[i + k][j + (WIN_SEQUENCE_SIZE - 1) - k] ==
                        diagonalKernL[k][(WIN_SEQUENCE_SIZE - 1) - k]) {
                        sum2++;
                    }
                }
                if (sum1 == WIN_SEQUENCE_SIZE || sum2 == WIN_SEQUENCE_SIZE)
                    return true;
            }
        }
    }

    return false;
}