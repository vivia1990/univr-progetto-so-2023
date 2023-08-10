#include "sem_api.h"
#include <stdio.h>

/**
 * Classe interna per operazioni su singolo semaforo
 */
static inline int sem_do_op(struct sm_semop *op);

static inline int
sem_do_op(struct sm_semop *op)
{
    return semop(op->id, &op->buf, 1);
}

int
sm_wait(int id, size_t index)
{
    struct sm_semop op = {.id = id};
    op.buf.sem_num = index;
    op.buf.sem_op = -1;
    op.buf.sem_flg = 0;

    if (sem_do_op(&op) < 0) {
        perror("sm_api: Errore sm_wait\n");
        fprintf(stderr, "%s %d %lu", __FUNCTION__, id, index);
        fflush(stderr);

        return -1;
    }

    return 1;
}

int
sm_signal(int id, size_t index)
{
    struct sm_semop op = {.id = id};
    op.buf.sem_num = index;
    op.buf.sem_op = 1;
    op.buf.sem_flg = 0;

    if (sem_do_op(&op) < 0) {
        perror("sm_api: Errore sm_signal\n");
        fprintf(stderr, "%s %d %lu", __FUNCTION__, id, index);
        fflush(stderr);

        return -1;
    }

    return 1;
}

int
sm_init(size_t count, u_int16_t *values)
{

    int semId = semget(IPC_PRIVATE, count, 0660);
    if (semId < 0) {
        perror("sm_api: Errore creazione semaforo\n");
        return -1;
    }

    union semun init;
    init.array = values;

    if (semctl(semId, 0, SETALL, init) < 0) {
        perror("sm_api: Errore inizializzazione semaforo\n");
        return -1;
    }

    return semId;
}

int
sm_rm(int id)
{
    return semctl(id, 0, IPC_RMID, 0);
}
