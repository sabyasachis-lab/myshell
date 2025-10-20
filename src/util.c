#include "util.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to set terminal to raw mode (disable line buffering)
void myshell_set_raw_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to restore terminal to normal mode
void myshell_restore_terminal() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);   // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

char myshell_take_input_char(){
    return getchar();
}

char* get_current_working_directory() {
    static char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return cwd;
    } else {
        return NULL;
    }
}

char* get_current_working_directory_home_shortened() {
    static char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Check if the current directory is under the home directory
        const char* home = getenv("HOME");
        if (home && strncmp(cwd, home, strlen(home)) == 0) {
            // Use a temporary buffer to avoid overlapping source and destination
            char temp[1024];
            snprintf(temp, sizeof(temp), "~%s", cwd + strlen(home));
            strcpy(cwd, temp);  // Copy back to the static buffer
        }
        return cwd;
    } else {
        return NULL;
    }
}
