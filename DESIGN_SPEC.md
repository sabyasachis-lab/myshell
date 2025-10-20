# MyShell - Design Specification (DS)

## 1. Architecture Overview

### 1.1 System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        MyShell                              │
├─────────────────────────────────────────────────────────────┤
│  main.c           │  Command Line Argument Processing       │
│                   │  Shell Initialization & Main Loop      │
├─────────────────────────────────────────────────────────────┤
│  myshell.c        │  Terminal Input Processing              │
│                   │  Command Execution & Signal Handling   │
├─────────────────────────────────────────────────────────────┤
│  builtin_commands │  Built-in Command Implementations      │
│  .c/.h            │  Command Handler Functions              │
├─────────────────────────────────────────────────────────────┤
│  hash_table.c/.h  │  Command Lookup Hash Table             │
│                   │  Hash Functions & Table Operations     │
├─────────────────────────────────────────────────────────────┤
│  util.c/.h        │  Utility Functions                     │
│                   │  Directory & String Operations         │
├─────────────────────────────────────────────────────────────┤
│  log.h            │  Logging Macros & Debug Support        │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Component Interaction

```
┌──────────┐    ┌─────────────┐    ┌──────────────┐    ┌──────────────┐
│   User   │───▶│   Terminal  │───▶│   MyShell    │───▶│   Commands   │
│  Input   │    │  Raw Mode   │    │   Parser     │    │   Execution  │
└──────────┘    └─────────────┘    └──────────────┘    └──────────────┘
                                           │
                                           ▼
                                   ┌──────────────┐
                                   │  Hash Table  │
                                   │   Lookup     │
                                   └──────────────┘
```

## 2. Module Design

### 2.1 Main Module (`main.c`)

#### 2.1.1 Purpose
Entry point for the shell, handles command line arguments and initializes the shell environment.

#### 2.1.2 Key Functions
```c
int main(int argc, char* argv[])
```
- **Input:** Command line arguments
- **Output:** Exit code (0 for success, non-zero for error)
- **Behavior:** Parses arguments, initializes shell, starts main loop

#### 2.1.3 Dependencies
- `myshell.h` - Core shell functionality
- `log.h` - Logging macros

### 2.2 Shell Core Module (`myshell.c/.h`)

#### 2.2.1 Purpose
Core shell functionality including input processing, tokenization, and command execution.

#### 2.2.2 Data Structures

```c
typedef struct term_input {
    char* buffer;                    // Input buffer
    unsigned int length;             // Current input length
    unsigned int token_count;        // Number of tokens parsed
    char* tokens[MYSHELL_MAX_TOKENS]; // Token array
} myshell_term_input_t;
```

#### 2.2.3 Key Functions

```c
void myshell_init_term_input(void)
```
- **Purpose:** Initialize terminal input system and hash table
- **Side Effects:** Sets terminal to raw mode, allocates buffers

```c
void myshell_process_input_char(char c)
```
- **Purpose:** Process individual character input
- **Input:** Character from terminal
- **Behavior:** Handles special keys, builds input buffer

```c
void myshell_process_buffer(void)
```
- **Purpose:** Process complete command line
- **Behavior:** Tokenizes input, looks up command, executes

```c
void myshell_extract_tokens_from_buffer(void)
```
- **Purpose:** Tokenize input buffer into command and arguments
- **Behavior:** Handles quoted strings, spaces, creates token array

#### 2.2.4 Terminal Control

```c
void myshell_set_raw_mode(void)
void myshell_restore_terminal(void)
```
- **Purpose:** Control terminal input mode
- **Implementation:** Uses `termios` for raw mode configuration

### 2.3 Hash Table Module (`hash_table.c/.h`)

#### 2.3.1 Purpose
Efficient command lookup using hash table data structure.

#### 2.3.2 Design Decisions

**Hash Table Size:** 128 entries (power of 2 for efficient modulo)
**Collision Resolution:** Simple chaining (future enhancement)
**Hash Functions:** Multiple implementations available

#### 2.3.3 Data Structures

```c
typedef struct myshell_hash_table {
    void* entries[MYSHELL_HASH_TABLE_SIZE];
} myshell_hash_table_t;
```

#### 2.3.4 Hash Functions

```c
unsigned int myshell_hash_string(const char* str)
```
- **Algorithm:** Simple additive hash
- **Purpose:** Default hash function for command names

```c
unsigned int myshell_hash_string_fnv(const char* str)
```
- **Algorithm:** FNV-1a hash
- **Purpose:** Alternative hash with better distribution

```c
unsigned int myshell_hash_string_poly(const char* str)
```
- **Algorithm:** Polynomial rolling hash
- **Purpose:** Cryptographically stronger hash

#### 2.3.5 Macro-Based Interface

```c
#define MYSHELL_HASH_TABLE_INIT(item_type, hash_table_ptr)
#define MYSHELL_HASH_TABLE_INSERT(item_type, hash_table_ptr, key, value_ptr)
#define MYSHELL_HASH_TABLE_LOOKUP(item_type, hash_table_ptr, key, result_ptr)
#define MYSHELL_HASH_TABLE_FREE(hash_table_ptr)
```

**Benefits:**
- Type-safe operations
- Compile-time error checking
- Easy to change implementation

### 2.4 Built-in Commands Module (`builtin_commands.c/.h`)

#### 2.4.1 Purpose
Implementation of all built-in shell commands.

#### 2.4.2 Command Handler Architecture

```c
typedef void (*myshell_command_handler_t)(const char* argv[]);

typedef struct builtin_command {
    const char* name;
    myshell_command_handler_t handler;
} myshell_builtin_command_t;
```

#### 2.4.3 Macro-Based Command Definition

```c
#define MYSHELL_COMMAND_HANDLER_SIGNATURE(name) void name(const char* argv[])
#define MYSHELL_DEFINE_COMMAND_HANDLER(name) MYSHELL_COMMAND_HANDLER_SIGNATURE(name)
#define MYSHELL_DECLARE_COMMAND_HANDLER(name) MYSHELL_COMMAND_HANDLER_SIGNATURE(name)
```

**Benefits:**
- Consistent function signatures
- Easy signature changes
- Reduced code duplication

#### 2.4.4 Command Registration System

```c
#define MYSHELL_LIST_BUILTIN_COMMANDS \
    X("help", myshell_cmd_help, "Show help message") \
    X("echo", myshell_cmd_echo, "Echo arguments") \
    X("ls", myshell_cmd_ls, "List directory") \
    // ... more commands
```

**X-Macro Pattern Benefits:**
- Single source of truth for command list
- Automatic array generation
- Easy to add new commands

#### 2.4.5 Command Implementations

**File System Commands:**
```c
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_ls)
```
- Uses `opendir()`, `readdir()`, `stat()` for directory listing
- Indicates file types with trailing characters (/, *, etc.)
- No external command execution for security

**Environment Commands:**
```c
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_set)
```
- Parses VARIABLE=value or VARIABLE value formats
- Uses `setenv()` for safer environment manipulation
- Proper memory management with temporary buffers

### 2.5 Utility Module (`util.c/.h`)

#### 2.5.1 Purpose
Common utility functions used across the shell.

#### 2.5.2 Key Functions

```c
char* get_current_working_directory(void)
```
- **Returns:** Current working directory path
- **Implementation:** Uses `getcwd()` with static buffer

```c
char* get_current_working_directory_home_shortened(void)
```
- **Returns:** CWD with $HOME replaced by ~
- **Implementation:** String manipulation with temporary buffer to avoid overlap

### 2.6 Logging Module (`log.h`)

#### 2.6.1 Purpose
Centralized logging and debugging support.

#### 2.6.2 Implementation

```c
#define MYSHELL_LOG(level, fmt, ...) \
    do { \
        if (level >= myshell_log_level) { \
            fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)
```

#### 2.6.3 Log Levels
- `MYSHELL_LOG_LEVEL_DEBUG` - Detailed execution tracing
- `MYSHELL_LOG_LEVEL_INFO` - General information
- `MYSHELL_LOG_LEVEL_WARN` - Warning conditions
- `MYSHELL_LOG_LEVEL_ERROR` - Error conditions
- `MYSHELL_LOG_LEVEL_NONE` - No logging (default)

## 3. Signal Handling Design

### 3.1 Signal Architecture

```c
void myshell_signal_handler(int sig)
void myshell_setup_signal_handlers(void)
```

### 3.2 Signal Processing

| Signal | Raw Mode Char | Action | Handler |
|--------|---------------|--------|---------|
| SIGINT | Ctrl+C (3) | Clear input | Continue shell |
| SIGTERM | - | Cleanup exit | Restore terminal |
| SIGQUIT | Ctrl+\ (28) | Force quit | Immediate exit |
| SIGTSTP | Ctrl+Z (26) | Message only | Continue shell |
| SIGPIPE | - | Ignore | Prevent crash |

### 3.3 Signal Safety
- Handlers use only async-signal-safe functions
- Global flag for signal communication
- Minimal processing in signal context

## 4. Memory Management

### 4.1 Memory Allocation Strategy

**Static Buffers:**
- Input buffer: 1024 bytes (prevents overflow)
- Working directory: 1024 bytes (PATH_MAX consideration)
- Token arrays: Fixed size for predictable memory usage

**Dynamic Allocation:**
- Hash table: Single malloc() for table structure
- Command parsing: Temporary allocations with proper cleanup

### 4.2 Memory Safety

**Buffer Overflow Prevention:**
- `strncat()` instead of `strcat()`
- `snprintf()` instead of `sprintf()`
- Bounds checking on all array access

**Memory Leak Prevention:**
- `free()` calls in error paths
- Cleanup functions for all modules
- Static analysis with compiler warnings

## 5. Build System Design

### 5.1 Makefile Architecture

```makefile
# Modular compilation
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Configurable build options
CFLAGS = -Wall -Wextra -std=c99 -g
```

### 5.2 Build Targets

| Target | Purpose | Features |
|--------|---------|----------|
| `all` | Default build | Standard compilation |
| `debug` | Debug build | Extra debug info, no optimization |
| `release` | Production build | Optimization, no debug info |
| `clean` | Cleanup | Remove artifacts and core files |

### 5.3 Debugging Support

**Core Dump Configuration:**
- Automatic core dump enablement
- Core file analysis targets
- GDB integration for debugging

**Compiler Flags:**
- `-Wall -Wextra` for comprehensive warnings
- `-g` for debug symbols
- `-std=c99` for standard compliance

## 6. Error Handling Strategy

### 6.1 Error Categories

**System Errors:**
- File operations: Use `perror()` for system error messages
- Memory allocation: Check return values, cleanup on failure
- Terminal operations: Restore state on error

**User Errors:**
- Invalid commands: Clear error messages with suggestions
- Malformed input: Graceful handling with usage information
- Permission errors: Appropriate error reporting

### 6.2 Error Recovery

**Graceful Degradation:**
- Continue operation after non-fatal errors
- Maintain shell state consistency
- Provide alternative paths when possible

**Cleanup Procedures:**
- Terminal state restoration
- Memory deallocation
- Signal handler cleanup

## 7. Testing Strategy

### 7.1 Unit Testing Approach

**Testable Components:**
- Hash functions (deterministic output)
- String utilities (input/output validation)
- Tokenization (various input scenarios)

**Mock Objects:**
- Terminal input simulation
- File system operation mocking
- Signal delivery simulation

### 7.2 Integration Testing

**End-to-End Scenarios:**
- Complete command execution flows
- Signal handling during operations
- Memory usage under load

**Stress Testing:**
- Long input sequences
- Rapid command execution
- Memory pressure scenarios

## 8. Performance Considerations

### 8.1 Optimization Areas

**Hash Table Performance:**
- O(1) average lookup time
- Efficient hash functions
- Minimal collision rate

**Input Processing:**
- Character-by-character processing
- Minimal buffering overhead
- Immediate user feedback

### 8.2 Memory Efficiency

**Static Allocation:**
- Predictable memory usage
- No fragmentation issues
- Fast allocation/deallocation

**Cache Efficiency:**
- Locality of reference in data structures
- Sequential access patterns where possible
- Minimal pointer chasing

## 9. Security Considerations

### 9.1 Input Validation

**Buffer Protection:**
- Strict length limits on all input
- Bounds checking on array operations
- Prevention of format string attacks

**Path Security:**
- Relative path validation
- Prevention of directory traversal
- Safe file operation practices

### 9.2 Privilege Management

**Minimal Privileges:**
- No unnecessary system calls
- User-level operations only
- Safe environment variable handling

## 10. Extensibility Design

### 10.1 Plugin Architecture (Future)

**Command Plugin Interface:**
```c
typedef struct command_plugin {
    const char* name;
    myshell_command_handler_t handler;
    const char* description;
    int (*init)(void);
    void (*cleanup)(void);
} myshell_command_plugin_t;
```

### 10.2 Configuration System (Future)

**Configuration File Format:**
```ini
[shell]
prompt = "myshell> "
history_size = 1000

[commands]
enable_ls = true
enable_cat = true
```

**Runtime Configuration:**
- Environment variable overrides
- Command-line option processing
- Dynamic feature enabling/disabling