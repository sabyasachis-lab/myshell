#ifndef MYSHELL_MAIN_H
#define MYSHELL_MAIN_H
#include <stdint.h>

#define MYSHELL_PROMPT_SYMBOL "> "
#define MYSHELL_BANNER \
    "MyShell - Simple Shell with Raw Input\n" \
    "Commands: 'exit' or 'quit' to exit, Ctrl+D to exit, Ctrl+C to clear input\n"


void myshell_parse_args(int argc, char* argv[]);
void myshell_setup_signal_handlers();
void myshell_init_term_input();
void myshell_show_banner();
void myshell_do_prompt_loop();
void myshell_abort(uint8_t exit_code);

#endif // MYSHELL_MAIN_H