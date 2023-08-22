#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define LOG_BUFFER_SIZE 1024

#define LOG_INFO(format, args...)                                              \
    do {                                                                       \
        print_log_message(info, format, args);                                 \
    } while (0);

#define LOG_ERROR(format, args...)                                             \
    do {                                                                       \
        print_log_message(error, format, args);                                \
    } while (0);

enum LogType { info, error, test_passed, test_failed, log_type_length };

struct Logger {
    int32_t outDescriptor;
    int32_t errorDescriptor;
};

// singleton, logger default
struct Logger *get_logger();

int32_t log_init(struct Logger *log, uint32_t outDesc, uint32_t errDesc);

void print_log_message(enum LogType type, const char *format, ...);

#endif