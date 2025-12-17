#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "builtin_commands.h"
#include "hash_table.h"

#define MYSHELL_MAX_INPUT_BUFFER_SIZE 1024
#define MYSHELL_MAX_TOKENS 64
#define MYSHELL_HISTORY_SIZE 100

typedef struct term_input {
    char* buffer;
    unsigned int length;
    unsigned int cursor_pos;  // Current cursor position in buffer
    unsigned int token_count;
    char* tokens[MYSHELL_MAX_TOKENS];
    char* redirect_file;      // File for redirection (NULL if none)
    bool redirect_append;     // true for >>, false for >
} myshell_term_input_t;

typedef struct command_history {
    char* entries[MYSHELL_HISTORY_SIZE];
    unsigned int count;          // Total entries added (may exceed HISTORY_SIZE)
    int current_index;           // Current position in history (-1 = not browsing)
    char* temp_buffer;           // Saved current input when browsing starts
} myshell_command_history_t;

//extern myshell_term_input_t myshell_term_input;
extern uint8_t myshell_log_level;

// Hash table for builtin commands  
extern myshell_hash_table_t* myshell_builtin_command_table_ptr;

// Command history functions
void myshell_history_init();
void myshell_history_add(const char* command);
void myshell_history_navigate_up();
void myshell_history_navigate_down();
void myshell_history_reset_navigation();
void myshell_history_save_to_file(const char* filepath);
void myshell_history_load_from_file(const char* filepath);
void myshell_clear_current_line();
void myshell_save_current_line(const char* line);
void myshell_restore_and_display_line(const char* line);

void myshell_show_prompt(bool newline);
void myshell_write_to_terminal(const char* format, ...);
void myshell_process_input_char(char c);
void myshell_clear_input_buffer();
void myshell_process_buffer();
void myshell_extract_tokens_from_buffer();
void myshell_signal_handler(int sig);
void myshell_show_usage(const char* program_name);

#endif // MYSHELL_H