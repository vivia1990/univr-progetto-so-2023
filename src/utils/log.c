#include <assert.h>
#include <errno.h>
#include <log.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ERRNO_LAB (ANSI_COLOR_RED "ERRNO" ANSI_COLOR_RESET ": ")

static const char *const LogLabels[log_type_length] = {
    [info] = ANSI_COLOR_BLUE "info" ANSI_COLOR_RESET,
    [error] = ANSI_COLOR_RED "error" ANSI_COLOR_RESET,
    [test_passed] = ANSI_COLOR_GREEN "test-passed" ANSI_COLOR_RESET,
    [test_failed] = ANSI_COLOR_RED "test-failed" ANSI_COLOR_RESET,
};

struct Logger *
get_logger()
{
    static struct Logger log = {};
    return &log;
}

int32_t
log_init(struct Logger *log, uint32_t outDesc, uint32_t errDesc)
{
    log->errorDescriptor = errDesc;
    log->outDescriptor = outDesc;

    return 1;
}

void
print_log_message(enum LogType type, const char *format, ...)
{
    _Bool errorOcc = errno != 0;
    const char *const err = strerror(errno);
    va_list args;

    char buff[LOG_BUFFER_SIZE];
    const size_t labLength = strlen(LogLabels[type]);
    memcpy(buff, LogLabels[type], labLength);
    buff[labLength] = ':';
    buff[labLength + 1] = ' ';

    va_start(args, format);
    ssize_t wb = vsnprintf(buff + labLength + 2, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    assert(wb < LOG_BUFFER_SIZE && wb > 0);

    wb += labLength + 2;
    assert(wb < LOG_BUFFER_SIZE);
    buff[wb] = '\n';

    wb++;
    buff[wb] = '\0';

    struct Logger *log = get_logger();
    _Bool isError = type == error;
    const int32_t fd = isError ? log->errorDescriptor : log->outDescriptor;

    if (isError && errorOcc) {
        const size_t length = strlen(err);
        memcpy(buff + wb, ERRNO_LAB, sizeof ERRNO_LAB);
        wb += sizeof ERRNO_LAB;
        assert(wb < LOG_BUFFER_SIZE);
        memcpy(buff + wb, err, length);
        wb += length;
        assert(wb < LOG_BUFFER_SIZE);
        buff[wb] = '\n';
    }

    wb++;
    buff[wb] = '\n';
    write(fd, buff, wb + 1);
}