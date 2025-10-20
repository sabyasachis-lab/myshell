#ifndef MYSHELL_UTIL_H
#define MYSHELL_UTIL_H

void myshell_set_raw_mode();
void myshell_restore_terminal();
char myshell_take_input_char();
char* get_current_working_directory();
char* get_current_working_directory_home_shortened();

#endif // MYSHELL_UTIL_H