#ifndef MYSHELL_LOG_H
#define MYSHELL_LOG_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MYSHELL_LOG_LEVEL_DEBUG 1
#define MYSHELL_LOG_LEVEL_INFO  2
#define MYSHELL_LOG_LEVEL_WARN  3
#define MYSHELL_LOG_LEVEL_ERROR 4
#define MYSHELL_LOG_LEVEL_NONE  5

#define MYSHELL_LOG_TYPE_CONSOLE 0
#define MYSHELL_LOG_TYPE_FILE    1

extern uint8_t myshell_log_level;
extern uint8_t myshell_log_type;
extern char* myshell_log_file_path;

// Global file handle for logging
extern FILE* myshell_log_file_handle;
extern bool myshell_log_initialized;

// Log level names for formatted output
static inline const char* myshell_log_level_name(uint8_t level) {
    switch(level) {
        case MYSHELL_LOG_LEVEL_DEBUG: return "DEBUG";
        case MYSHELL_LOG_LEVEL_INFO:  return "INFO";
        case MYSHELL_LOG_LEVEL_WARN:  return "WARN";
        case MYSHELL_LOG_LEVEL_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Initialize logging (open file if needed)
#define MYSHELL_LOG_INIT() \
    do { \
        if (!myshell_log_initialized && myshell_log_type == MYSHELL_LOG_TYPE_FILE) { \
            if (myshell_log_file_path != NULL) { \
                myshell_log_file_handle = fopen(myshell_log_file_path, "a"); \
                if (!myshell_log_file_handle) { \
                    fprintf(stderr, "Warning: Failed to open log file: %s\n", myshell_log_file_path); \
                } \
            } else { \
                fprintf(stderr, "Warning: Log file path not set\n"); \
            } \
            myshell_log_initialized = true; \
        } \
    } while(0)

// Unified logging macro
#define MYSHELL_LOG(level, fmt, ...) \
    do { \
        if (level >= myshell_log_level) { \
            MYSHELL_LOG_INIT(); \
            if (myshell_log_type == MYSHELL_LOG_TYPE_FILE) { \
                if (myshell_log_file_handle) { \
                    time_t now = time(NULL); \
                    struct tm* tm_info = localtime(&now); \
                    char time_buf[64]; \
                    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info); \
                    fprintf(myshell_log_file_handle, "[%s] [%s] " fmt "\n", \
                            time_buf, myshell_log_level_name(level), ##__VA_ARGS__); \
                    fflush(myshell_log_file_handle); \
                } \
            } else { \
                fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
            } \
        } \
    } while(0)

#endif // MYSHELL_LOG_H