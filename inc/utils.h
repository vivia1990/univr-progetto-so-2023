#ifndef UTILS_H
#define UTILS_H

#include "log.h"
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PANIC(msg, status, args...)                                            \
    do {                                                                       \
        LOG_ERROR(msg, args);                                                  \
        exit(status);                                                          \
    } while (0);

char *int_to_string(int32_t value, char *buff, size_t length);
ssize_t create_fifo(const char *path);

#endif