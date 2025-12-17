#define _POSIX_C_SOURCE 200809L  // Enable POSIX signal features
#include "util.h"
#include "log.h"
#include "myshell.h"
#include "main.h"
#include "hash_table.h"
#include "external_commands.h"
#include "output_redirection.h"
#include <signal.h>
#include <string.h>  // for strlen, strcmp
#include <stdlib.h>  // for malloc, free, exit
#include <stdio.h>   // for printf, fprintf
#include <stdarg.h>  // for va_list, va_start, va_end
// Global variable definition
myshell_term_input_t myshell_term_input;
myshell_command_history_t myshell_history;
uint8_t myshell_log_level = MYSHELL_LOG_LEVEL_NONE; // Default log level
uint8_t myshell_log_type = MYSHELL_LOG_TYPE_CONSOLE; // Default to console logging
char* myshell_log_file_path = NULL; // Log file path
FILE* myshell_log_file_handle = NULL; // Log file handle
bool myshell_log_initialized = false; // Log initialization flag
myshell_hash_table_t* myshell_builtin_command_table_ptr = NULL; // Initialize to NULL

// Global flag for signal handling
volatile sig_atomic_t signal_received = 0;


// Function to show usage information
void myshell_show_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOPTIONS:\n");
    printf("  -v <LOG_TYPE>    Enable verbose logging with specified type\n");
    printf("                   LOG_TYPE: CONSOLE (default) or FILE\n");
    printf("  -f <FILE_PATH>   Specify log file path (required when -v FILE)\n");
    printf("  -h, --help       Show this help message and exit\n");
    printf("  --version        Show version information and exit\n");
    printf("\nEXAMPLES:\n");
    printf("  %s                      Start shell with no logging\n", program_name);
    printf("  %s -v CONSOLE           Start with console logging\n", program_name);
    printf("  %s -v FILE -f mylog.log Start with file logging\n", program_name);
    printf("\nINTERACTIVE COMMANDS:\n");
    printf("  exit, quit       Exit the shell\n");
    printf("  Ctrl+D           Exit the shell\n");
    printf("  Ctrl+C           Clear current input line\n");
    printf("\n");
}

// Function to parse command line arguments
void myshell_parse_args(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            // Enable verbose logging with log type
            if (i + 1 < argc) {
                i++;
                if (strcmp(argv[i], "CONSOLE") == 0) {
                    myshell_log_level = MYSHELL_LOG_LEVEL_DEBUG;
                    myshell_log_type = MYSHELL_LOG_TYPE_CONSOLE;
                    MYSHELL_LOG(MYSHELL_LOG_LEVEL_INFO, "Console logging enabled");
                } else if (strcmp(argv[i], "FILE") == 0) {
                    myshell_log_level = MYSHELL_LOG_LEVEL_DEBUG;
                    myshell_log_type = MYSHELL_LOG_TYPE_FILE;
                    // Log file path should be specified with -f
                } else {
                    fprintf(stderr, "Error: Invalid log type '%s'. Use CONSOLE or FILE.\n", argv[i]);
                    fprintf(stderr, "Use '%s --help' for usage information.\n", argv[0]);
                    exit(1);
                }
            } else {
                fprintf(stderr, "Error: -v option requires a log type (CONSOLE or FILE)\n");
                fprintf(stderr, "Use '%s --help' for usage information.\n", argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-f") == 0) {
            // Specify log file path
            if (i + 1 < argc) {
                i++;
                myshell_log_file_path = argv[i];
            } else {
                fprintf(stderr, "Error: -f option requires a file path\n");
                fprintf(stderr, "Use '%s --help' for usage information.\n", argv[0]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            myshell_show_usage(argv[0]);
            exit(0);
        }
        else if (strcmp(argv[i], "--version") == 0) {
            printf("MyShell version 1.0.0\n");
            printf("A simple shell with raw terminal input processing\n");
            exit(0);
        }
        else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Use '%s --help' for usage information.\n", argv[0]);
            exit(1);
        }
    }
    
    // Validate that if FILE logging is selected, a file path is provided
    if (myshell_log_type == MYSHELL_LOG_TYPE_FILE && myshell_log_file_path == NULL) {
        fprintf(stderr, "Error: FILE logging requires -f <file_path> option\n");
        fprintf(stderr, "Use '%s --help' for usage information.\n", argv[0]);
        exit(1);
    }
    
    // Log the file path if file logging is enabled
    if (myshell_log_type == MYSHELL_LOG_TYPE_FILE) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_INFO, "File logging enabled: %s", myshell_log_file_path);
    }
}

// Function to setup signal handlers
void myshell_setup_signal_handlers() {
    struct sigaction sa;
    
    // Initialize sigaction structure
    sa.sa_handler = myshell_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls
    
    // Setup handlers for various signals
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(1);
    }
    
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(1);
    }
    
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction SIGTSTP");
        exit(1);
    }
    
    // Ignore SIGPIPE (broken pipe)
    signal(SIGPIPE, SIG_IGN);
}

// Signal handler function
void myshell_signal_handler(int sig) {
    switch(sig) {
        case SIGINT:  // Ctrl+C
            printf("\n[Signal SIGINT received - use Ctrl+D in input or 'exit' to quit]\n");
            break;
        case SIGTERM: // Termination signal
            printf("\n[SIGTERM received - cleaning up]\n");
            myshell_abort(0);
            break;
        case SIGQUIT: // Ctrl+backslash
            printf("\n[SIGQUIT received - cleaning up]\n");
            break;
        case SIGTSTP: // Ctrl+Z
            printf("\n[SIGTSTP received - shell cannot be suspended in raw mode]\n");
            break;
        default:
            printf("\n[Unknown signal %d received]\n", sig);
            break;
    }
    signal_received = sig;
    myshell_show_prompt(false);
}

void myshell_register_builtin_commands() {
    MYSHELL_HASH_TABLE_INIT(myshell_builtin_command_t, myshell_builtin_command_table_ptr);
    for (int i = 0; myshell_builtin_commands[i].name != NULL; i++) {
        MYSHELL_HASH_TABLE_INSERT(myshell_builtin_command_t, myshell_builtin_command_table_ptr, myshell_builtin_commands[i].name, &myshell_builtin_commands[i]);
    }
}

// Initialize command history
void myshell_history_init() {
    myshell_history.count = 0;
    myshell_history.current_index = -1;
    myshell_history.temp_buffer = NULL;
    for (int i = 0; i < MYSHELL_HISTORY_SIZE; i++) {
        myshell_history.entries[i] = NULL;
    }
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Command history initialized");
}

// Add a command to history
void myshell_history_add(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }
    
    // Don't add duplicate of last command
    if (myshell_history.count > 0) {
        unsigned int last_idx = (myshell_history.count - 1) % MYSHELL_HISTORY_SIZE;
        if (myshell_history.entries[last_idx] && strcmp(myshell_history.entries[last_idx], command) == 0) {
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Skipping duplicate command in history");
            return;
        }
    }
    
    unsigned int idx = myshell_history.count % MYSHELL_HISTORY_SIZE;
    
    // Free old entry if it exists
    if (myshell_history.entries[idx] != NULL) {
        free(myshell_history.entries[idx]);
    }
    
    // Allocate and copy new entry
    myshell_history.entries[idx] = strdup(command);
    if (myshell_history.entries[idx] == NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_ERROR, "Failed to allocate memory for history entry");
        return;
    }
    
    myshell_history.count++;
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Added to history [%u]: %s", myshell_history.count - 1, command);
}

// Navigate up in history (older commands)
void myshell_history_navigate_up() {
    if (myshell_history.count == 0) {
        return;  // No history
    }
    
    // If starting navigation, save current input
    if (myshell_history.current_index == -1) {
        myshell_save_current_line(myshell_term_input.buffer);
        // Start at most recent command
        myshell_history.current_index = myshell_history.count - 1;
    } else {
        // Move to older command
        unsigned int oldest_available = (myshell_history.count >= MYSHELL_HISTORY_SIZE) ?
                                        (myshell_history.count - MYSHELL_HISTORY_SIZE) : 0;
        if ((unsigned int)myshell_history.current_index > oldest_available) {
            myshell_history.current_index--;
        } else {
            // Already at oldest, don't move
            return;
        }
    }
    
    // Load history entry into buffer
    unsigned int idx = myshell_history.current_index % MYSHELL_HISTORY_SIZE;
    if (myshell_history.entries[idx] != NULL) {
        // Clear current line on screen
        myshell_clear_current_line();
        
        // Restore and display history entry
        myshell_restore_and_display_line(myshell_history.entries[idx]);
        
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Navigated to history [%d]: %s", 
                    myshell_history.current_index, myshell_term_input.buffer);
    }
}

// Navigate down in history (newer commands)
void myshell_history_navigate_down() {
    if (myshell_history.current_index == -1) {
        return;  // Not browsing history
    }
    
    // Move to newer command
    myshell_history.current_index++;
    
    // Clear current line on screen
    myshell_clear_current_line();
    
    // Check if we're back at the original input
    if ((unsigned int)myshell_history.current_index >= myshell_history.count) {
        // Restore original input
        if (myshell_history.temp_buffer != NULL) {
            myshell_restore_and_display_line(myshell_history.temp_buffer);
            free(myshell_history.temp_buffer);
            myshell_history.temp_buffer = NULL;
        } else {
            myshell_term_input.buffer[0] = '\0';
            myshell_term_input.length = 0;
            myshell_term_input.cursor_pos = 0;
        }
        myshell_history.current_index = -1;
        
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Restored original input: %s", myshell_term_input.buffer);
    } else {
        // Load history entry
        unsigned int idx = myshell_history.current_index % MYSHELL_HISTORY_SIZE;
        if (myshell_history.entries[idx] != NULL) {
            myshell_restore_and_display_line(myshell_history.entries[idx]);
            
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Navigated to history [%d]: %s", 
                        myshell_history.current_index, myshell_term_input.buffer);
        }
    }
}

// Reset history navigation state
void myshell_history_reset_navigation() {
    myshell_history.current_index = -1;
    if (myshell_history.temp_buffer != NULL) {
        free(myshell_history.temp_buffer);
        myshell_history.temp_buffer = NULL;
    }
}

// Save command history to file
void myshell_history_save_to_file(const char* filepath) {
    FILE* file = fopen(filepath, "w");
    if (file == NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_WARN, "Failed to save history to file: %s", filepath);
        return;
    }
    
    // Determine the range of valid entries to save
    unsigned int start_idx = 0;
    unsigned int num_entries = myshell_history.count;
    
    if (num_entries > MYSHELL_HISTORY_SIZE) {
        // We have wrapped around, save only the last MYSHELL_HISTORY_SIZE entries
        start_idx = myshell_history.count - MYSHELL_HISTORY_SIZE;
        num_entries = MYSHELL_HISTORY_SIZE;
    }
    
    // Write entries in chronological order (oldest to newest)
    for (unsigned int i = 0; i < num_entries; i++) {
        unsigned int idx = (start_idx + i) % MYSHELL_HISTORY_SIZE;
        if (myshell_history.entries[idx] != NULL) {
            fprintf(file, "%s\n", myshell_history.entries[idx]);
        }
    }
    
    fclose(file);
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Saved %u history entries to %s", num_entries, filepath);
}

// Load command history from file
void myshell_history_load_from_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "No history file found at: %s", filepath);
        return;
    }
    
    char line[MYSHELL_MAX_INPUT_BUFFER_SIZE];
    unsigned int loaded_count = 0;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Add to history (don't skip duplicates on load)
        if (strlen(line) > 0) {
            unsigned int idx = myshell_history.count % MYSHELL_HISTORY_SIZE;
            
            // Free old entry if it exists
            if (myshell_history.entries[idx] != NULL) {
                free(myshell_history.entries[idx]);
            }
            
            // Allocate and copy new entry
            myshell_history.entries[idx] = strdup(line);
            if (myshell_history.entries[idx] != NULL) {
                myshell_history.count++;
                loaded_count++;
            }
        }
    }
    
    fclose(file);
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Loaded %u history entries from %s", loaded_count, filepath);
}

// Clear the current line on screen
void myshell_clear_current_line() {
    // Move cursor to beginning of current line
    for (unsigned int i = 0; i < myshell_term_input.cursor_pos; i++) {
        myshell_write_to_terminal("\b");
    }
    // Overwrite with spaces
    for (unsigned int i = 0; i < myshell_term_input.length; i++) {
        myshell_write_to_terminal(" ");
    }
    // Move cursor back to beginning
    for (unsigned int i = 0; i < myshell_term_input.length; i++) {
        myshell_write_to_terminal("\b");
    }
}

// Save the current line content to buffer
void myshell_save_current_line(const char* line) {
    if (myshell_history.temp_buffer != NULL) {
        free(myshell_history.temp_buffer);
    }
    myshell_history.temp_buffer = strdup(line);
}

// Restore line content to buffer and display it
void myshell_restore_and_display_line(const char* line) {
    // Copy to buffer
    strncpy(myshell_term_input.buffer, line, MYSHELL_MAX_INPUT_BUFFER_SIZE - 1);
    myshell_term_input.buffer[MYSHELL_MAX_INPUT_BUFFER_SIZE - 1] = '\0';
    myshell_term_input.length = strlen(myshell_term_input.buffer);
    myshell_term_input.cursor_pos = myshell_term_input.length;
    
    // Display the line
    myshell_write_to_terminal("%s", myshell_term_input.buffer);
}

void myshell_init_term_input(){
    // allocate buffer
    myshell_term_input.buffer = (char*)malloc(MYSHELL_MAX_INPUT_BUFFER_SIZE);
    if (myshell_term_input.buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    myshell_clear_input_buffer();
    // Initialize history
    myshell_history_init();
    // Load history from file
    const char* home = getenv("HOME");
    if (home != NULL) {
        char history_path[1024];
        snprintf(history_path, sizeof(history_path), "%s/.myshell_history", home);
        myshell_history_load_from_file(history_path);
    }
    // set terminal raw mode
    myshell_set_raw_mode();
    // register built-in commands
    myshell_register_builtin_commands();
}

void myshell_abort(uint8_t exit_code) {
    if(exit_code == 0) {
        printf("See you again soon...\n");
    } else {
        printf("Program ended with exit code %d\n", exit_code);
    }
    myshell_restore_terminal();
    
    // Save history to file
    const char* home = getenv("HOME");
    if (home != NULL) {
        char history_path[1024];
        snprintf(history_path, sizeof(history_path), "%s/.myshell_history", home);
        myshell_history_save_to_file(history_path);
    }
    
    // Free history entries
    for (int i = 0; i < MYSHELL_HISTORY_SIZE; i++) {
        if (myshell_history.entries[i] != NULL) {
            free(myshell_history.entries[i]);
        }
    }
    if (myshell_history.temp_buffer != NULL) {
        free(myshell_history.temp_buffer);
    }
    
    MYSHELL_HASH_TABLE_FREE(myshell_builtin_command_table_ptr);
    exit(exit_code);
}

// Wrapper function to write to terminal
void myshell_write_to_terminal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void myshell_clear_input_buffer(){
    // clear input buffer
    memset(myshell_term_input.buffer, 0, MYSHELL_MAX_INPUT_BUFFER_SIZE);
    // reset length
    myshell_term_input.length = 0;
    // reset cursor position
    myshell_term_input.cursor_pos = 0;
    // reset token count
    myshell_term_input.token_count = 0;
    // clear tokens
    for(int i = 0; i < MYSHELL_MAX_TOKENS; i++) {
        myshell_term_input.tokens[i] = NULL;
    }
    // reset redirection
    myshell_term_input.redirect_file = NULL;
    myshell_term_input.redirect_append = false;
}


// Function to process each character
void myshell_process_input_char(char c) {
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "|0x%02x ('%c')|", c, (c >= 32 && c <= 126) ? c : ' '); // Debug print
    
    // Reset history navigation on any character except arrows
    if (c != 27 && myshell_history.current_index != -1) {
        // User started typing, exit history browsing mode
        myshell_history_reset_navigation();
    }
    
    switch(c) {
        case '\n':
        case '\r':
            if(strlen(myshell_term_input.buffer) == 0) {
                myshell_show_prompt(true);
                return;
            }
            printf("\n");
            // Add to history before processing
            myshell_history_add(myshell_term_input.buffer);
            // Reset history navigation
            myshell_history_reset_navigation();
            myshell_process_buffer();
            myshell_clear_input_buffer();
            myshell_show_prompt(true);
            break;
        case 127:  // Backspace (DEL)
        case 8:    // Backspace (BS)
            if (myshell_term_input.cursor_pos > 0) {
                // If cursor is at the end, simple backspace
                if (myshell_term_input.cursor_pos == myshell_term_input.length) {
                    myshell_write_to_terminal("\b \b");
                    myshell_term_input.buffer[myshell_term_input.cursor_pos - 1] = '\0';
                    myshell_term_input.length--;
                    myshell_term_input.cursor_pos--;
                } else {
                    // Cursor in the middle - need to shift characters left
                    // Move cursor back one position
                    myshell_write_to_terminal("\b");
                    
                    // Shift characters left in buffer
                    for (unsigned int i = myshell_term_input.cursor_pos - 1; i < myshell_term_input.length - 1; i++) {
                        myshell_term_input.buffer[i] = myshell_term_input.buffer[i + 1];
                    }
                    myshell_term_input.buffer[myshell_term_input.length - 1] = '\0';
                    myshell_term_input.length--;
                    myshell_term_input.cursor_pos--;
                    
                    // Redraw from cursor to end
                    myshell_write_to_terminal("%s ", &myshell_term_input.buffer[myshell_term_input.cursor_pos]);
                    
                    // Move cursor back to correct position
                    unsigned int chars_after = myshell_term_input.length - myshell_term_input.cursor_pos;
                    for (unsigned int i = 0; i <= chars_after; i++) {
                        myshell_write_to_terminal("\b");
                    }
                }
            }
            break;
        case 27:  // Escape sequences (arrows, etc.)
            {
                char seq1 = getchar();
                char seq2 = getchar();
                
                if (seq1 == '[') {
                    switch(seq2) {
                        case 'D':  // Left arrow
                            if (myshell_term_input.cursor_pos > 0) {
                                myshell_write_to_terminal("\b");  // Move cursor left
                                myshell_term_input.cursor_pos--;
                                MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Cursor moved left to position %u", myshell_term_input.cursor_pos);
                            }
                            break;
                        case 'C':  // Right arrow
                            if (myshell_term_input.cursor_pos < myshell_term_input.length) {
                                myshell_write_to_terminal("%c", myshell_term_input.buffer[myshell_term_input.cursor_pos]);  // Move cursor right
                                myshell_term_input.cursor_pos++;
                                MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Cursor moved right to position %u", myshell_term_input.cursor_pos);
                            }
                            break;
                        case 'A':  // Up arrow - navigate to older command
                            myshell_history_navigate_up();
                            break;
                        case 'B':  // Down arrow - navigate to newer command
                            myshell_history_navigate_down();
                            break;
                    }
                }
            }
            return;
        case 9:  // Tab
            myshell_write_to_terminal("[TAB]");
            return;  // Ignore for now
        case 0:  // Null character
            return;  // Ignore
        default:
            // Only accept printable characters
            if (c >= 32 && c <= 126) {
                if(myshell_term_input.length >= MYSHELL_MAX_INPUT_BUFFER_SIZE - 1) {
                    // print the char in RED color to indicate buffer full
                    myshell_write_to_terminal("\033[31m%c\033[0m", c); // Red color
                } else {
                    // If cursor is at the end, simple append
                    if (myshell_term_input.cursor_pos == myshell_term_input.length) {
                        myshell_write_to_terminal("%c", c);
                        myshell_term_input.buffer[myshell_term_input.cursor_pos] = c;
                        myshell_term_input.buffer[myshell_term_input.cursor_pos + 1] = '\0';
                        myshell_term_input.length++;
                        myshell_term_input.cursor_pos++;
                    } else {
                        // Cursor in the middle - need to insert character
                        // Shift characters right in buffer
                        for (unsigned int i = myshell_term_input.length; i > myshell_term_input.cursor_pos; i--) {
                            myshell_term_input.buffer[i] = myshell_term_input.buffer[i - 1];
                        }
                        myshell_term_input.buffer[myshell_term_input.cursor_pos] = c;
                        myshell_term_input.buffer[myshell_term_input.length + 1] = '\0';
                        myshell_term_input.length++;
                        myshell_term_input.cursor_pos++;
                        
                        // Redraw from cursor to end
                        myshell_write_to_terminal("%s", &myshell_term_input.buffer[myshell_term_input.cursor_pos - 1]);
                        
                        // Move cursor back to correct position
                        unsigned int chars_after = myshell_term_input.length - myshell_term_input.cursor_pos;
                        for (unsigned int i = 0; i < chars_after; i++) {
                            myshell_write_to_terminal("\b");
                        }
                    }
                }
            }
            break;
    }
}

void myshell_process_buffer() {
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "\nBuffer content: %s\n", myshell_term_input.buffer);
    myshell_extract_tokens_from_buffer();
    fflush(stdout);

    // Setup output redirection if needed
    myshell_redirect_state_t redirect_state = 
        myshell_setup_output_redirection(myshell_term_input.redirect_file, 
                                         myshell_term_input.redirect_append);
    
    // If redirection was requested but failed, return early
    if (myshell_term_input.redirect_file != NULL && redirect_state.saved_stdout == -1) {
        return;
    }

    // 1. Try builtin commands first (hash table lookup)
    myshell_builtin_command_t *builtin_cmd = NULL;
    MYSHELL_HASH_TABLE_LOOKUP(myshell_builtin_command_t, myshell_builtin_command_table_ptr, myshell_term_input.tokens[0], builtin_cmd);
    if (builtin_cmd != NULL && builtin_cmd->handler != NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Executing builtin command handler for: %s", myshell_term_input.tokens[0]);
        builtin_cmd->handler((const char**)myshell_term_input.tokens);
    }
    // 2. Try external command execution
    else {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Not a builtin command, trying external execution");
        int result = myshell_execute_external_command(myshell_term_input.tokens);
        if (result < 0) {
            // Restore stdout before printing error (if redirected)
            myshell_restore_output_redirection(&redirect_state);
            // Command not found
            printf("Error: Unknown command '%s'\n", myshell_term_input.tokens[0]);
            return;
        } else if (result != 0) {
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "External command exited with code: %d", result);
        }
    }
    
    // Restore stdout if it was redirected
    myshell_restore_output_redirection(&redirect_state);
}

void myshell_extract_tokens_from_buffer() {
    // Placeholder for extracting tokens from the input buffer
    // This function can be expanded to tokenize the input command
    // For now, just print a debug message
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Extracting tokens from buffer: %s", myshell_term_input.buffer);
    bool in_quotes = false;
    bool token_start = true;
    for(unsigned int i = 0; i < myshell_term_input.length; i++) {
        // If quote found, toggle in_quotes flag
        if(myshell_term_input.buffer[i] == '\"' || myshell_term_input.buffer[i] == '\'') {
            in_quotes = !in_quotes;
        }
        // If token_start flag in ON, mark the token start in the input string
        if(token_start){
            myshell_term_input.tokens[myshell_term_input.token_count] = &myshell_term_input.buffer[i];
            token_start = false;
        }
        // If space found, this may be an end of a token
        if(myshell_term_input.buffer[i] == ' ') {
            // If we are not in quotes, this is a token boundary
            if(!in_quotes) {
                myshell_term_input.buffer[i] = '\0'; // Null-terminate tokens
                myshell_term_input.token_count++;
                token_start = true;
                // If we exceed max tokens, stop processing
                if(myshell_term_input.token_count >= MYSHELL_MAX_TOKENS) {
                    MYSHELL_LOG(MYSHELL_LOG_LEVEL_WARN, "Maximum token limit reached (%d)", MYSHELL_MAX_TOKENS);
                    break;
                }
            } else {
                // Inside quotes, treat space as normal character
                continue;
            }
        } else {
            // Normal character, continue
            continue;
        }
    }
    // If we ended while still in a token, count it
    if(!token_start) {
        myshell_term_input.token_count++;
    }
    if(myshell_term_input.token_count > MYSHELL_MAX_TOKENS) {
        myshell_term_input.token_count = MYSHELL_MAX_TOKENS;
    }
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Total tokens extracted: %u", myshell_term_input.token_count);
    
    // Check for output redirection (> or >>)
    if (myshell_term_input.token_count >= 2) {
        unsigned int redirect_idx = myshell_term_input.token_count - 2;
        const char* redirect_op = myshell_term_input.tokens[redirect_idx];
        
        if (strcmp(redirect_op, ">") == 0 || strcmp(redirect_op, ">>") == 0) {
            // Found redirection operator
            myshell_term_input.redirect_file = myshell_term_input.tokens[redirect_idx + 1];
            myshell_term_input.redirect_append = (strcmp(redirect_op, ">>") == 0);
            
            // Remove redirection operator and filename from command tokens
            myshell_term_input.tokens[redirect_idx] = NULL;
            myshell_term_input.token_count = redirect_idx;
            
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Output redirection: %s %s", 
                       redirect_op, myshell_term_input.redirect_file);
        }
    }
    
    // Print the tokens for debugging
    for(unsigned int i = 0; i < myshell_term_input.token_count; i++) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Token[%u]: %s", i, myshell_term_input.tokens[i]);
    }
}

void myshell_show_banner(){
    printf(MYSHELL_BANNER);
}

void myshell_show_prompt(bool newline){
    if (newline) {
        printf("\n");
    }
    printf("%s ", get_current_working_directory_home_shortened());
    printf(MYSHELL_PROMPT_SYMBOL);
    fflush(stdout);
}



void myshell_do_prompt_loop(){
    char c;
    // Show first prompt
    myshell_show_prompt(false);
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Entering main input loop");
    while (1) {
        // Read one character without waiting for Enter
        c = myshell_take_input_char();
        // Process the character
        myshell_process_input_char(c);
    }
}