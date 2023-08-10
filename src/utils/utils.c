#include "headers.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

char *
int_to_string(int32_t value, char *buff, size_t length)
{
    snprintf(buff, length, "%d", value);
    return buff;
}

ssize_t
create_fifo(const char *path)
{
    unlink(path);
    return mkfifo(path, 0660);
}