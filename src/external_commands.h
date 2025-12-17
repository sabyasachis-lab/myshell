#ifndef MYSHELL_EXTERNAL_COMMANDS_H
#define MYSHELL_EXTERNAL_COMMANDS_H

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// External command execution
int myshell_resolve_binary_path(const char* command, char* resolved_path);
int myshell_execute_external_command(char* const argv[]);

#endif // MYSHELL_EXTERNAL_COMMANDS_H
