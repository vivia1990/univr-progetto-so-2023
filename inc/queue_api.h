#ifndef QUEUE_API_H
#define QUEUE_API_H

#include "messages.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct Payload {
    int64_t mtype;
    char payload[256];
};

/**
 * Manda un messaggio di tipo connection
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 */
int32_t queue_send_connection(int32_t qId, const void *msg, size_t msgSize,
                              uint32_t msgType);

/**
 * Riceve un messaggio di tipo connection
 * @param msg puntatore alla struttura del messagio
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 */
int32_t queue_recive_connection(int32_t qId, void *msg, size_t msgSize,
                                uint32_t msgType);

/**
 * Manda un messaggio di tipo game
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 * @param messageType tipo messaggio
 */
int32_t queue_send_game(int32_t qId, const void *msg, size_t msgSize,
                        int32_t messageType);

/**
 * Riceve un messaggio di tipo game
 * @param msg puntatore alla struttura del messagio
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 * @param messageType tipo messaggio
 */
int32_t queue_recive_game(int32_t qId, void *msg, size_t msgSize,
                          int32_t messageType);

/**
 * Manda un messaggio di tipo erro
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 */
int32_t queue_send_error(int32_t qId, const struct ErrorMsg *msg);

/**
 * Riceve un messaggio di tipo game
 * @param msg puntatore alla struttura del messagio
 * @param msgSize dimensione in byte della struttura senza contare il campo
 * mtype
 * @param messageType tipo messaggio
 */
int32_t queue_recive_error(int32_t qId, struct ErrorMsg *msg);

_Bool queue_is_empty(int32_t qId);

#endif