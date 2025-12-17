#define _POSIX_C_SOURCE 200809L  // Enable POSIX functions

#include "external_commands.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Resolve binary path by searching CWD and BINPATH
 * @param command Command name to search for
 * @param resolved_path Output buffer for resolved path (PATH_MAX size)
 * @return 0 on success, -1 if not found
 */
int myshell_resolve_binary_path(const char* command, char* resolved_path) {
    if (!command || !resolved_path) {
        return -1;
    }
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Resolving binary path for: %s", command);
    
    // 1. If command contains '/', treat as path (absolute/relative)
    if (strchr(command, '/') != NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Command contains '/', treating as path");
        if (access(command, X_OK) == 0) {
            if (realpath(command, resolved_path) != NULL) {
                MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Resolved to: %s", resolved_path);
                return 0;
            }
        }
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Path not found or not executable");
        return -1;
    }
    
    // 2. Check current working directory
    char cwd_path[PATH_MAX];
    snprintf(cwd_path, PATH_MAX, "./%s", command);
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Checking CWD: %s", cwd_path);
    
    if (access(cwd_path, X_OK) == 0) {
        if (realpath(cwd_path, resolved_path) != NULL) {
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Found in CWD: %s", resolved_path);
            return 0;
        }
    }
    
    // 3. Search in BINPATH (colon-separated)
    const char* binpath = getenv("BINPATH");
    if (binpath == NULL) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "BINPATH not set");
        return -1;
    }
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Searching in BINPATH: %s", binpath);
    
    char* binpath_copy = strdup(binpath);
    if (!binpath_copy) {
        perror("strdup");
        return -1;
    }
    
    char* saveptr;
    char* dir = strtok_r(binpath_copy, ":", &saveptr);
    
    while (dir != NULL) {
        snprintf(resolved_path, PATH_MAX, "%s/%s", dir, command);
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Trying: %s", resolved_path);
        
        if (access(resolved_path, X_OK) == 0) {
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Found in BINPATH: %s", resolved_path);
            free(binpath_copy);
            return 0;
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }
    
    free(binpath_copy);
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Binary not found in BINPATH");
    return -1;
}

/**
 * Execute external command using fork/exec
 * @param argv Null-terminated argument array
 * @return 0 on success, exit code of child process, or -1 on failure
 */
int myshell_execute_external_command(char* const argv[]) {
    if (!argv || !argv[0]) {
        return -1;
    }
    
    char resolved_path[PATH_MAX];
    
    // Resolve binary path
    if (myshell_resolve_binary_path(argv[0], resolved_path) != 0) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Failed to resolve binary path for: %s", argv[0]);
        return -1; // Binary not found
    }
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Executing external command: %s", resolved_path);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // Child process: execute the binary
        execv(resolved_path, argv);
        
        // If execv returns, it failed
        perror("execv");
        exit(127); // Standard exit code for command not found
    }
    
    // Parent process: wait for child
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Command exited with code: %d", exit_code);
        return exit_code;
    }
    
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Command terminated by signal %d\n", sig);
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Command terminated by signal: %d", sig);
        return 128 + sig;
    }
    
    return -1;
}
