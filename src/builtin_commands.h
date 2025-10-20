#ifndef BUILTIN_COMMANDS_H
#define BUILTIN_COMMANDS_H


// Macro to define command handler function signature
// Change this single line to modify ALL command handler signatures
#define MYSHELL_COMMAND_HANDLER_SIGNATURE(name) void name(const char* argv[])

// Function pointer type using the macro
typedef MYSHELL_COMMAND_HANDLER_SIGNATURE((*myshell_command_handler_t));

// Convenience macros for declaring and defining command handlers
#define MYSHELL_DECLARE_COMMAND_HANDLER(name) MYSHELL_COMMAND_HANDLER_SIGNATURE(name)
#define MYSHELL_DEFINE_COMMAND_HANDLER(name) MYSHELL_COMMAND_HANDLER_SIGNATURE(name)

typedef struct builtin_command {
    const char* name;
    myshell_command_handler_t handler;
} myshell_builtin_command_t;

extern myshell_builtin_command_t myshell_builtin_commands[];

#define MYSHELL_LIST_BUILTIN_COMMANDS \
    X("help", myshell_cmd_help, "Show this help message") \
    X("echo", myshell_cmd_echo, "Echo arguments to stdout") \
    X("version", myshell_cmd_version, "Show version information") \
    X("clear", myshell_cmd_clear, "Clear the screen") \
    X("exit", myshell_cmd_exit, "Exit the shell") \
    X("quit", myshell_cmd_exit, "Exit the shell") \
    X("cd", myshell_cmd_cd, "Change directory") \
    X("pwd", myshell_cmd_pwd, "Print working directory") \
    X("set", myshell_cmd_set, "Set environment variable") \
    X("unset", myshell_cmd_unset, "Unset environment variable") \
    X("env", myshell_cmd_env, "List environment variables") \
    X("ls", myshell_cmd_ls, "List directory contents") \
    X("cat", myshell_cmd_cat, "Concatenate and display file contents") \
    X("touch", myshell_cmd_touch, "Create an empty file or update timestamp")

#define X(name, handler, description) MYSHELL_DECLARE_COMMAND_HANDLER(handler);
MYSHELL_LIST_BUILTIN_COMMANDS
#undef X

#endif // BUILTIN_COMMANDS_H