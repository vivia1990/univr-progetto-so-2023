#include "log.h"
#include "server.h"
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static void
sig_int_handler()
{
    struct Server *server = get_server();
    if (!server->wasCtrlCPressed) {
        server->wasCtrlCPressed = true;
    }
    else {
        LOG_INFO("CTRL-C premuto ripetutamente, terminazione Server", "")
        down_server(server);
        exit(EXIT_SUCCESS);
    }
}

static void
sig_usr_handler(int32_t signal)
{
    struct Server *server = get_server();
    const size_t sender = signal == SIGUSR2; // 0 or 1
    server->players[sender]->disconnected = true;
    ++server->disconnectionCounter;
    server->disconnectionHappened = true;
}

static void
sig_alarm_handler(int32_t signal)
{
    struct Server *server = get_server();
    server->timeoutHappened = true;
}

int32_t
server_init_signals()
{
    sigset_t signals;
    sigfillset(&signals);

    sigdelset(&signals, SIGINT);
    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);
    sigdelset(&signals, SIGALRM);
    if (sigprocmask(SIG_SETMASK, &signals, NULL) < 0) {
        LOG_ERROR("Errore set procmask server", "")
    }

    if (signal(SIGINT, sig_int_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGINT", "")
    }
    if (signal(SIGUSR1, sig_usr_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGUSR1", "")
    }
    if (signal(SIGUSR2, sig_usr_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGUSR2", "")
    }
    if (signal(SIGALRM, sig_alarm_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGUSR2", "")
    }

    return 1;
}