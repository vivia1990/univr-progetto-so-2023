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
        _exit(status);                                                         \
    } while (0);

char *int_to_string(int32_t value, char *buff, size_t length);
ssize_t create_fifo(const char *path);
ssize_t create_queue();
ssize_t remove_queue(int32_t queueId);

/**
 * ANSI escape sequence to clear most of linux terminals
 */
void clear_terminal();

int32_t stop_file_block(int32_t fileDesc);

int32_t resume_file_block(int32_t fileDesc);

#endif