#include "server.h"
#include "utils.h"
#include <stdint.h>
#include <sys/msg.h>
/*
 * Crea e inizializza e mette in STOPPED il gestore delle connessioni
 * Deve essere chiamato dal processo server
 */
int32_t conn_create_manager(struct Server *server);

/*
 * Dealloca le strutture inizializzate dal manager
 */
int32_t conn_remove_manager(struct Server *server);

/*
 * Riprende l'ascolto delle connessioni l'ascolto per le connessioni
 */
int32_t conn_resume_listening(struct Server *server);

/*
 * Mette in pausa -Stopped- il processo e inizia l'ascolto per le connessioni
 */
int32_t conn_pause_listening(struct Server *server);
