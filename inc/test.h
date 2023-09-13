#include "log.h"
#include <assert.h>
#include <unistd.h>

#define TEST_PASSED(format, args...)                                           \
    do {                                                                       \
        char __test_h_passed = '\n';                                           \
        write(STDOUT_FILENO, &__test_h_passed, 1);                             \
        print_log_message(test_passed, format, args);                          \
        write(STDOUT_FILENO, &__test_h_passed, 1);                             \
    } while (0);

#define TEST_FAILED(format, args...)                                           \
    do {                                                                       \
        char __test_h_failed = '\n';                                           \
        write(STDOUT_FILENO, &__test_h_failed, 1);                             \
        print_log_message(test_failed, format, args);                          \
        write(STDOUT_FILENO, &__test_h_failed, 1);                             \
    } while (0);