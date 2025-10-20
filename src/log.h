#ifndef MYSHELL_LOG_H
#define MYSHELL_LOG_H

#include <stdio.h>
#include <stdint.h>

#define MYSHELL_LOG_LEVEL_DEBUG 1
#define MYSHELL_LOG_LEVEL_INFO  2
#define MYSHELL_LOG_LEVEL_WARN  3
#define MYSHELL_LOG_LEVEL_ERROR 4
#define MYSHELL_LOG_LEVEL_NONE  5

extern uint8_t myshell_log_level;

#define MYSHELL_LOG(level, fmt, ...) \
    do { \
        if (level >= myshell_log_level) { \
            fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

#endif // MYSHELL_LOG_H