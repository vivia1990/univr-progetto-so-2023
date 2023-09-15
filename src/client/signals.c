#include "client.h"
#include "log.h"
#include "utils.h"
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

static void
sig_int_handler()
{
    struct Client *client = get_client();
    kill(client->serverPid, client->signalDisconnection);
    down_client(client);
}

static void
sig_alarm_handler(int32_t signal)
{
    struct Client *client = get_client();
    client->timeoutHappened = true;
    stop_file_block(STDIN_FILENO);
}

int32_t
client_init_signals()
{
    sigset_t signals;
    sigfillset(&signals);

    sigdelset(&signals, SIGINT);
    sigdelset(&signals, SIGALRM);
    if (sigprocmask(SIG_SETMASK, &signals, NULL) < 0) {
        LOG_ERROR("Errore set procmask server", "")
    }

    if (signal(SIGINT, sig_int_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGINT", "")
    }
    if (signal(SIGALRM, sig_alarm_handler) == SIG_ERR) {
        LOG_ERROR("Errore set handler SIGUSR2", "")
    }

    return 1;
}