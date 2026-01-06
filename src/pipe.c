#define _POSIX_C_SOURCE 200809L
#include "pipe.h"
#include "log.h"
#include "external_commands.h"
#include "builtin_commands.h"
#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

extern myshell_hash_table_t* myshell_builtin_command_table_ptr;

// Parse tokens into pipeline structure by splitting on '|'
int myshell_parse_pipeline(char** tokens, unsigned int token_count, myshell_pipeline_t* pipeline) {
    pipeline->command_count = 0;
    unsigned int current_cmd = 0;
    unsigned int token_start = 0;
    
    for (unsigned int i = 0; i <= token_count; i++) {
        // Check if we hit a pipe or end of tokens
        if (i == token_count || (tokens[i] != NULL && strcmp(tokens[i], "|") == 0)) {
            if (i > token_start) {
                // We have tokens for a command
                if (current_cmd >= MYSHELL_MAX_PIPE_COMMANDS) {
                    MYSHELL_LOG(MYSHELL_LOG_LEVEL_ERROR, "Too many piped commands (max %d)", MYSHELL_MAX_PIPE_COMMANDS);
                    return -1;
                }
                
                pipeline->commands[current_cmd].tokens = &tokens[token_start];
                pipeline->commands[current_cmd].token_count = i - token_start;
                
                // Null terminate the command tokens
                if (i < token_count) {
                    tokens[i] = NULL;
                }
                
                current_cmd++;
                token_start = i + 1;
            }
        }
    }
    
    pipeline->command_count = current_cmd;
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Parsed %u commands in pipeline", pipeline->command_count);
    
    return pipeline->command_count;
}

// Execute a single command in the pipeline context
static int execute_command_in_pipeline(char** tokens, unsigned int token_count, int input_fd, int output_fd) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        
        // Redirect input if needed
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        
        // Redirect output if needed
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        // Check if it's a builtin command
        myshell_builtin_command_t *builtin_cmd = NULL;
        MYSHELL_HASH_TABLE_LOOKUP(myshell_builtin_command_t, myshell_builtin_command_table_ptr, tokens[0], builtin_cmd);
        
        if (builtin_cmd != NULL && builtin_cmd->handler != NULL) {
            // Execute builtin in child process
            builtin_cmd->handler((const char**)tokens);
            exit(0);
        } else {
            // Try external command
            char resolved_path[4096];
            if (myshell_resolve_binary_path(tokens[0], resolved_path) != 0) {
                fprintf(stderr, "Error: Command not found '%s'\n", tokens[0]);
                exit(127);
            }
            
            // Execute external command
            execv(resolved_path, tokens);
            perror("execv");
            exit(1);
        }
    }
    
    // Parent process
    return pid;
}

// Execute a pipeline of commands
int myshell_execute_pipeline(myshell_pipeline_t* pipeline) {
    if (pipeline->command_count == 0) {
        return -1;
    }
    
    // Single command - no pipes needed
    if (pipeline->command_count == 1) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Single command, executing directly");
        // This shouldn't normally be called for single commands, but handle it anyway
        return myshell_execute_external_command(pipeline->commands[0].tokens);
    }
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Executing pipeline with %u commands", pipeline->command_count);
    
    int prev_pipe_read = STDIN_FILENO;
    pid_t* pids = malloc(sizeof(pid_t) * pipeline->command_count);
    
    if (pids == NULL) {
        perror("malloc");
        return -1;
    }
    
    for (unsigned int i = 0; i < pipeline->command_count; i++) {
        int pipe_fds[2];
        
        // Create pipe for all but the last command
        if (i < pipeline->command_count - 1) {
            if (pipe(pipe_fds) < 0) {
                perror("pipe");
                free(pids);
                return -1;
            }
        }
        
        // Determine input and output file descriptors
        int input_fd = (i == 0) ? STDIN_FILENO : prev_pipe_read;
        int output_fd = (i == pipeline->command_count - 1) ? STDOUT_FILENO : pipe_fds[1];
        
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Executing command %u: %s", i, pipeline->commands[i].tokens[0]);
        
        // Execute the command
        pid_t pid = execute_command_in_pipeline(
            pipeline->commands[i].tokens,
            pipeline->commands[i].token_count,
            input_fd,
            output_fd
        );
        
        if (pid < 0) {
            free(pids);
            return -1;
        }
        
        pids[i] = pid;
        
        // Close the write end of the pipe in parent
        if (i < pipeline->command_count - 1) {
            close(pipe_fds[1]);
        }
        
        // Close the previous read end if it's not stdin
        if (prev_pipe_read != STDIN_FILENO) {
            close(prev_pipe_read);
        }
        
        // Save the read end for the next iteration
        if (i < pipeline->command_count - 1) {
            prev_pipe_read = pipe_fds[0];
        }
    }
    
    // Wait for all child processes
    int last_status = 0;
    for (unsigned int i = 0; i < pipeline->command_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            last_status = WEXITSTATUS(status);
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Command %u exited with status %d", i, last_status);
        }
    }
    
    free(pids);
    return last_status;
}
