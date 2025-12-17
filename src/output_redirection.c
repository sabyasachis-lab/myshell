#define _POSIX_C_SOURCE 200809L
#include "output_redirection.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Setup output redirection (> or >>)
myshell_redirect_state_t myshell_setup_output_redirection(const char* filename, bool append) {
    myshell_redirect_state_t state;
    state.saved_stdout = -1;
    state.redirect_fp = NULL;
    
    if (filename == NULL) {
        return state;
    }
    
    // Save original stdout
    state.saved_stdout = dup(STDOUT_FILENO);
    if (state.saved_stdout == -1) {
        perror("dup");
        return state;
    }
    
    // Open the redirect file
    const char* mode = append ? "a" : "w";
    FILE* fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("fopen");
        fprintf(stderr, "Error: Cannot open file '%s' for writing\n", filename);
        close(state.saved_stdout);
        state.saved_stdout = -1;
        return state;
    }
    
    // Redirect stdout to the file
    if (dup2(fileno(fp), STDOUT_FILENO) == -1) {
        perror("dup2");
        fclose(fp);
        close(state.saved_stdout);
        state.saved_stdout = -1;
        return state;
    }
    
    state.redirect_fp = (void*)fp;
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Redirecting output to: %s (%s mode)",
               filename, mode);
    
    return state;
}

// Restore stdout to original state
void myshell_restore_output_redirection(myshell_redirect_state_t* state) {
    if (state == NULL || state->saved_stdout == -1) {
        return;
    }
    
    // Flush stdout before restoring
    fflush(stdout);
    
    // Restore original stdout
    dup2(state->saved_stdout, STDOUT_FILENO);
    close(state->saved_stdout);
    
    // Close the redirect file
    if (state->redirect_fp != NULL) {
        fclose((FILE*)state->redirect_fp);
        state->redirect_fp = NULL;
    }
    
    state->saved_stdout = -1;
}
