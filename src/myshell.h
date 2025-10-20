#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "builtin_commands.h"
#include "hash_table.h"

#define MYSHELL_MAX_INPUT_BUFFER_SIZE 1024
#define MYSHELL_MAX_TOKENS 64

typedef struct term_input {
    char* buffer;
    unsigned int length;
    unsigned int token_count;
    char* tokens[MYSHELL_MAX_TOKENS];
} myshell_term_input_t;

//extern myshell_term_input_t myshell_term_input;
extern uint8_t myshell_log_level;

// Hash table for builtin commands  
extern myshell_hash_table_t* myshell_builtin_command_table_ptr;

void myshell_show_prompt(bool newline);
void myshell_process_input_char(char c);
void myshell_clear_input_buffer();
void myshell_process_buffer();
void myshell_extract_tokens_from_buffer();
void myshell_signal_handler(int sig);
void myshell_show_usage(const char* program_name);

#endif // MYSHELL_H