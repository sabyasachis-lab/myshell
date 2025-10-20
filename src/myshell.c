#define _POSIX_C_SOURCE 200809L  // Enable POSIX signal features
#include "util.h"
#include "log.h"
#include "myshell.h"
#include "main.h"
#include "hash_table.h"
#include <signal.h>
#include <string.h>  // for strlen, strcmp
#include <stdlib.h>  // for malloc, free, exit
#include <stdio.h>   // for printf, fprintf
// Global variable definition
myshell_term_input_t myshell_term_input;
uint8_t myshell_log_level = MYSHELL_LOG_LEVEL_NONE; // Default log level
myshell_hash_table_t* myshell_builtin_command_table_ptr = NULL; // Initialize to NULL

// Global flag for signal handling
volatile sig_atomic_t signal_received = 0;


// Function to show usage information
void myshell_show_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOPTIONS:\n");
    printf("  -v, --verbose    Enable verbose (DEBUG) logging\n");
    printf("  -h, --help       Show this help message and exit\n");
    printf("  --version        Show version information and exit\n");
    printf("\nEXAMPLES:\n");
    printf("  %s              Start shell with normal logging\n", program_name);
    printf("  %s -v           Start shell with debug logging\n", program_name);
    printf("  %s --verbose    Start shell with debug logging\n", program_name);
    printf("\nINTERACTIVE COMMANDS:\n");
    printf("  exit, quit       Exit the shell\n");
    printf("  Ctrl+D           Exit the shell\n");
    printf("  Ctrl+C           Clear current input line\n");
    printf("\n");
}

// Function to parse command line arguments
void myshell_parse_args(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            myshell_log_level = MYSHELL_LOG_LEVEL_DEBUG;
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_INFO, "Verbose logging enabled");
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

void myshell_init_term_input(){
    // allocate buffer
    myshell_term_input.buffer = (char*)malloc(MYSHELL_MAX_INPUT_BUFFER_SIZE);
    if (myshell_term_input.buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    myshell_clear_input_buffer();
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
    MYSHELL_HASH_TABLE_FREE(myshell_builtin_command_table_ptr);
    exit(exit_code);
}

void myshell_clear_input_buffer(){
    // clear input buffer
    memset(myshell_term_input.buffer, 0, MYSHELL_MAX_INPUT_BUFFER_SIZE);
    // reset length
    myshell_term_input.length = 0;
    // reset token count
    myshell_term_input.token_count = 0;
    // clear tokens
    for(int i = 0; i < MYSHELL_MAX_TOKENS; i++) {
        myshell_term_input.tokens[i] = NULL;
    }
}


// Function to process each character
void myshell_process_input_char(char c) {
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Processing char: 0x%02x ('%c')\n", c, (c >= 32 && c <= 126) ? c : ' '); // Debug print
    switch(c) {
        case '\n':
        case '\r':
            if(strlen(myshell_term_input.buffer) == 0) {
                myshell_show_prompt(true);
                return;
            }
            printf("\n");
            myshell_process_buffer();
            myshell_clear_input_buffer();
            myshell_show_prompt(true);
            break;
        case 127:  // Backspace (DEL)
        case 8:    // Backspace (BS)
            if (myshell_term_input.length > 0) {
                // Move cursor back, print space, move back again
                printf("\b \b");
                fflush(stdout);
                // Remove last character from buffer
                myshell_term_input.buffer[myshell_term_input.length - 1] = '\0';
                myshell_term_input.length--;
            }
            break;
        case 27:  // Escape sequences (arrows, etc.)
            // Consume next two characters for escape sequence
            getchar(); getchar();
            return;  // Ignore for now
        case 9:  // Tab
            printf("[TAB]");
            fflush(stdout);
            return;  // Ignore for now
        case 0:  // Null character
            return;  // Ignore
        default:
            // Only accept printable characters
            if (c >= 32 && c <= 126) {
                if(myshell_term_input.length == MYSHELL_MAX_INPUT_BUFFER_SIZE - 1) {
                    // print the char in RED color to indicate buffer full
                    printf("\033[31m%c\033[0m", c); // Red color
                    fflush(stdout);
                } else {
                    printf("%c", c);
                    fflush(stdout);
                    strncat(myshell_term_input.buffer, &c, 1);
                    myshell_term_input.length++;
                }
            }
            break;
    }
}

void myshell_process_buffer() {
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "\nBuffer content: %s\n", myshell_term_input.buffer);
    myshell_extract_tokens_from_buffer();
    fflush(stdout);

    myshell_builtin_command_t *builtin_cmd = NULL;
    MYSHELL_HASH_TABLE_LOOKUP(myshell_builtin_command_t, myshell_builtin_command_table_ptr, myshell_term_input.tokens[0], builtin_cmd);
    if (builtin_cmd != NULL && builtin_cmd->handler != NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Executing command handler for: %s", myshell_term_input.tokens[0]);
        builtin_cmd->handler((const char**)myshell_term_input.tokens);
    } else {
        printf("Error: Unknown command '%s'\n", myshell_term_input.tokens[0]);
    }
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