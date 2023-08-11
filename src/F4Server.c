#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char const *argv[])
{
    log_init(get_logger(), STDOUT_FILENO, STDERR_FILENO);

    return EXIT_SUCCESS;
}