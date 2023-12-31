#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "messages.h"
#include "server.h"
#include "utils.h"
#include <stdint.h>

/*
 * Crea e inizializza e mette in STOPPED il gestore delle connessioni
 * Deve essere chiamato dal processo server
 */
int32_t conn_init_manager(struct Server *server, struct ServerArgs *args);

/*
 * Dealloca le strutture inizializzate dal manager
 */
int32_t conn_remove_manager(struct ConnectionManager *manager);

/*
 * Riprende l'ascolto delle connessioni l'ascolto per le connessioni
 */
int32_t conn_resume_listening(struct ConnectionManager *manager);

/*
 * Mette in pausa -Stopped- il processo e inizia l'ascolto per le connessioni
 */
int32_t conn_pause_listening(struct ConnectionManager *manager);

/*
 * Crea una struttura di tipo client (malloc) dalla request di connessione
 */
struct Client *create_client(struct ClientConnectionRequest *request,
                             char symbol);

#endif