#include <stddef.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/types.h>

struct sm_semop {
    int id;            // ipc key
    struct sembuf buf; // operation
};

union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
};

/**
 * Decrementa il semaforo di 1, blocca il processo se sem <= 0
 * @param id ipc key
 * @param index sem index from 0 to n - 1
 * @return > 0 se ok
 */
int sm_wait(int id, size_t index);

/**
 * Incrementa il semaforo di 1
 * @param id ipc key
 * @param index sem index from 0 to n - 1
 * @return > 0 se ok
 */
int sm_signal(int id, size_t index);

/**
 * Crea un set di count semafori inizializzati a values
 * @param count numero di semafori da creare
 * @param index valori di init, la dimensione deve essere uguale a count - 1
 * @return > 0 se ok
 */
int sm_init(size_t count, u_int16_t *values);

/**
 * Rimuove un set di semafori
 * @param id id del set
 * @return > 0 se ok
 */
int sm_rm(int id);