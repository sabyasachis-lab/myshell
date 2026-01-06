#ifndef PIPE_H
#define PIPE_H

#define MYSHELL_MAX_PIPE_COMMANDS 16

// Structure to hold a single command in a pipeline
typedef struct {
    char** tokens;              // Command tokens
    unsigned int token_count;   // Number of tokens
} myshell_pipe_command_t;

// Structure to hold parsed pipeline information
typedef struct {
    myshell_pipe_command_t commands[MYSHELL_MAX_PIPE_COMMANDS];
    unsigned int command_count;
} myshell_pipeline_t;

// Parse tokens into pipeline structure
// Returns the number of commands in the pipeline
int myshell_parse_pipeline(char** tokens, unsigned int token_count, myshell_pipeline_t* pipeline);

// Execute a pipeline of commands
// Returns 0 on success, -1 on failure
int myshell_execute_pipeline(myshell_pipeline_t* pipeline);

#endif // PIPE_H
