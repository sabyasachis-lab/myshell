#define _POSIX_C_SOURCE 200809L  // Enable POSIX functions

#include "myshell.h"
#include "main.h"
#include "log.h"
#include "util.h"
#include <stdio.h>   // for printf, fflush, fopen, fgets
#include <stdlib.h>  // for atoi, exit, putenv
#include <unistd.h>  // for chdir, unsetenv
#include <string.h>  // for snprintf
#include <dirent.h>  // for opendir, readdir, closedir
#include <sys/stat.h> // for stat, lstat
#include <errno.h>   // for errno

#define X(name, handler, description) {name, handler},
myshell_builtin_command_t myshell_builtin_commands[] = {
    MYSHELL_LIST_BUILTIN_COMMANDS
    {NULL, NULL}  // Sentinel to mark end of array
};
#undef X

// Handler for 'help' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_help) {
    printf("Available commands:\n");
#define X(name, handler, description) printf("  %-8s - %s\n", name, description);
    MYSHELL_LIST_BUILTIN_COMMANDS
#undef X
    
    if (argv && argv[0]) {
        MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Help command called with args starting with: %s", argv[0]);
    }
}

// Handler for 'echo' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_echo) {
    if (!argv) {
        printf("\n");
        return;
    }
    
    // Skip the command name (argv[0]) and print the rest
    for (int i = 1; argv[i] != NULL; i++) {
        if (i > 1) printf(" ");  // Add space between arguments
        printf("%s", argv[i]);
    }
    printf("\n");
}

// Handler for 'version' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_version) {
    (void)argv; // Suppress unused parameter warning
    printf("MyShell version 1.0.0\n");
    printf("A simple shell with raw terminal input processing\n");
    printf("Built with command handler support\n");
}

// Handler for 'clear' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_clear) {
    (void)argv; // Suppress unused parameter warning
    // ANSI escape sequence to clear screen and move cursor to top-left
    printf("\033[2J\033[H");
    fflush(stdout);
}

// Handler for 'exit' or 'quit' commands
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_exit) {
    if (argv && argv[1]) {
        // If exit code is provided, use it
        int exit_code = atoi(argv[1]);
        printf("Exiting with code %d\n", exit_code);
        myshell_abort(exit_code);
    } else {
        printf("Goodbye!\n");
        myshell_abort(0);
    }
}

// Handler for 'cd' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_cd) {
    if (argv && argv[1]) {
        if (chdir(argv[1]) == 0) {
            MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Changed directory to: %s", argv[1]);
        } else {
            perror("cd");
        }
    } else {
        fprintf(stderr, "cd: missing operand\n");
    }
}

// Handler for 'pwd' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_pwd) {
    (void)argv; // Suppress unused parameter warning
    char* cwd = get_current_working_directory();
    if (cwd != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

// Handler for 'set' command  
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_set) {
    if (!argv || !argv[1]) {
        printf("Usage: set VARIABLE=value\n");
        return;
    }
    
    // Parse VARIABLE=value format
    char* assignment = strdup(argv[1]);  // Make a copy to modify
    if (!assignment) {
        perror("set: memory allocation failed");
        return;
    }
    char* equals = strchr(assignment, '=');
    if (!equals) {
        printf("set: Invalid format. Use VARIABLE=value\n");
        free(assignment);
        return;
    }
    
    // Split into variable name and value
    *equals = '\0';  // Null-terminate the variable name
    char* variable = assignment;
    char* value = equals + 1;
    
    if (setenv(variable, value, 1) != 0) {  // 1 = overwrite existing
        perror("set");
    }
    
    free(assignment);
}

// Handler for 'unset' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_unset) {
    if (!argv || !argv[1]) {
        printf("Usage: unset VARIABLE\n");
        return;
    }
    if (unsetenv(argv[1]) != 0) {
        perror("unset");
    }
}

// Handler for 'env' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_env) {
    (void)argv; // Suppress unused parameter warning
    extern char **environ;
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

// Handler for 'ls' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_ls) {
    const char* path = (argv && argv[1]) ? argv[1] : ".";
    
    DIR* dir = opendir(path);
    if (!dir) {
        perror("ls");
        return;
    }
    
    struct dirent* entry;
    struct stat file_stat;
    char full_path[1024];
    
    printf("Contents of %s:\n", path);
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files starting with '.' unless we want to show them
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        // Create full path for stat
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &file_stat) == 0) {
            // Check if it's a directory
            if (S_ISDIR(file_stat.st_mode)) {
                printf("  %s/\n", entry->d_name);  // Directory with trailing slash
            } else if (S_ISREG(file_stat.st_mode)) {
                printf("  %s\n", entry->d_name);   // Regular file
            } else {
                printf("  %s*\n", entry->d_name);  // Other file types with asterisk
            }
        } else {
            // If stat fails, just print the name
            printf("  %s\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

// Handler for 'cat' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_cat) {
    if (!argv || !argv[1]) {
        printf("Usage: cat filename\n");
        return;
    }
    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("cat");
        return;
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    fclose(file);
}

// Handler for 'touch' command
MYSHELL_DEFINE_COMMAND_HANDLER(myshell_cmd_touch) {
    if (!argv || !argv[1]) {
        printf("Usage: touch filename\n");
        return;
    }
    
    FILE* file = fopen(argv[1], "a");
    if (!file) {
        perror("touch");
        return;
    }
    fclose(file);
}