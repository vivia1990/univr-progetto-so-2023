#include "connection_manager.h"
#include "log.h"
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    struct Server *server = get_server();
    init_server(server);
    LOG_INFO("Server started, pid: %d", server->pid);

    conn_create_manager(server);
    if (conn_resume_listening(server) < 0) {
        LOG_ERROR("Errore resume connection manager", "")
    }

    LOG_INFO("waiting for players.., connectionManagerPid: %d",
             server->connServicePid);
    waitpid(server->connServicePid, NULL, WSTOPPED);

    LOG_INFO("player connected game starts in few seconds ", "");
    // log players;

    print_server(server);

    down_server(server);
    return EXIT_SUCCESS;
}

struct Server *
get_server()
{
    static struct Server server = {0};
    return &server;
}