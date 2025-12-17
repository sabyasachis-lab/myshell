# MyShell Logging System

## Overview

MyShell supports two logging modes that can be selected at compile time:

1. **STDERR Mode** (default) - Logs printed to stderr
2. **File Mode** - Logs written to a file

## Logging Modes

### STDERR Mode (Default)

When `MYSHELL_LOG_FILE` is **not defined**, logs are printed to stderr.

**Build:**
```bash
make clean && make
```

**Run with verbose logging:**
```bash
./mysh -v
```

**Characteristics:**
- Logs printed to stderr in real-time
- Simple format: `message`
- No timestamps
- Best for debugging during development

---

### File Logging Mode

When `MYSHELL_LOG_FILE` is **defined**, logs are written exclusively to a file.

**Build:**
```bash
# Option 1: Use Makefile target (logs to ./logs/myshell.log)
make log-file

# Option 2: Custom log file path
make clean
make CFLAGS="-Wall -Wextra -std=c99 -g -DMYSHELL_LOG_FILE='\"/path/to/logfile.log\"'"
```

**Run:**
```bash
./mysh -v
```

**Characteristics:**
- Logs written to file only (stderr is clean)
- Formatted with timestamps: `[YYYY-MM-DD HH:MM:SS] [LEVEL] message`
- File opened once at initialization (efficient)
- File handle persists for session duration
- Best for production monitoring

---

## Log Levels

Available log levels (set with `-v` flag or in code):

- `MYSHELL_LOG_LEVEL_DEBUG` (1) - Detailed debug information
- `MYSHELL_LOG_LEVEL_INFO` (2) - Informational messages
- `MYSHELL_LOG_LEVEL_WARN` (3) - Warning messages
- `MYSHELL_LOG_LEVEL_ERROR` (4) - Error messages
- `MYSHELL_LOG_LEVEL_NONE` (5) - No logging (default)

## Implementation Details

### File Initialization

The log file is opened only once using the `MYSHELL_LOG_INIT()` macro:

```c
#define MYSHELL_LOG_INIT() \
    do { \
        if (!myshell_log_initialized) { \
            myshell_log_file_handle = fopen(MYSHELL_LOG_FILE, "a"); \
            if (!myshell_log_file_handle) { \
                fprintf(stderr, "Warning: Failed to open log file: %s\n", MYSHELL_LOG_FILE); \
            } \
            myshell_log_initialized = true; \
        } \
    } while(0)
```

- Called automatically by `MYSHELL_LOG()` on first use
- Uses static boolean flag to prevent repeated initialization
- Fails gracefully if file cannot be opened

### Log Macro Behavior

**STDERR Mode:**
```c
#define MYSHELL_LOG(level, fmt, ...) \
    do { \
        if (level >= myshell_log_level) { \
            fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)
```

**File Mode:**
```c
#define MYSHELL_LOG(level, fmt, ...) \
    do { \
        if (level >= myshell_log_level) { \
            MYSHELL_LOG_INIT(); \
            if (myshell_log_file_handle) { \
                // Write formatted log with timestamp \
                fprintf(myshell_log_file_handle, "[timestamp] [LEVEL] " fmt "\n", ...); \
                fflush(myshell_log_file_handle); \
            } \
        } \
    } while(0)
```

## Examples

### Example 1: Development with stderr logging
```bash
make clean && make
./mysh -v
> whoami
> date
> exit
```

Output goes to stderr with debug information visible.

### Example 2: Production with file logging
```bash
make log-file
./mysh -v &
# Shell output is clean, logs go to ./logs/myshell.log
tail -f logs/myshell.log  # Monitor logs in separate terminal
```

### Example 3: Custom log location
```bash
make clean
make CFLAGS="-Wall -Wextra -std=c99 -g -DMYSHELL_LOG_FILE='\"/var/log/myshell.log\"'"
sudo ./mysh -v  # May need sudo for /var/log access
```

## Testing

Run comprehensive logging tests:
```bash
./test_logging_modes.sh
```

This script tests:
- STDERR mode functionality
- File mode functionality  
- Single file handle initialization
- Log entry formats
- Both logging modes in sequence

## Makefile Targets

```bash
make help  # See all available targets including:
           # - all: Default build (stderr mode)
           # - log-file: Build with file logging
```
