#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PANIC(msg, status)                                                     \
    do {                                                                       \
        perror(msg);                                                           \
        exit(status);                                                          \
    } while (0);

char *int_to_string(int32_t value, char *buff, size_t length);
ssize_t create_fifo(const char *path);