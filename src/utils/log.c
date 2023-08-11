#include <assert.h>
#include <log.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char *const LogLabels[log_type_length] = {
    [info] = ANSI_COLOR_BLUE "info" ANSI_COLOR_RESET,
    [error] = ANSI_COLOR_RED "error" ANSI_COLOR_RESET,
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
    buff[wb] = '\n';
    buff[wb + 1] = '\0';

    struct Logger *log = get_logger();
    const int32_t fd =
        type == error ? log->errorDescriptor : log->outDescriptor;

    write(fd, buff, wb + 1);
}