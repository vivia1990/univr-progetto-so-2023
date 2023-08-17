#include "server.h"
#include <stdint.h>
#include <sys/queue.h>

/*
 * Crea e inizializza e mette in STOPPED il gestore delle connessioni
 * Deve essere chiamato dal processo server
 */
int32_t conn_create_manager(struct Server *server);

/*
 * Stoppa il processo e inizia l'ascolto per le connessioni
 */
int32_t conn_stop_listening();

/*
 * Mette in pausa -Stopped- il processo e inizia l'ascolto per le connessioni
 */
int32_t conn_stop_listening();
