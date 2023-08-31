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

int32_t
server_init_signals()
{
    sigset_t signals;
    sigfillset(&signals);

    sigdelset(&signals, SIGINT);
    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);
    sigprocmask(SIG_SETMASK, &signals, NULL);

    signal(SIGINT, sig_int_handler);
    signal(SIGUSR1, sig_usr_handler);
    signal(SIGUSR2, sig_usr_handler);

    return 1;
}