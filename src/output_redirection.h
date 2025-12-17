#ifndef OUTPUT_REDIRECTION_H
#define OUTPUT_REDIRECTION_H

#include <stdbool.h>

// Structure to hold redirection state
typedef struct {
    int saved_stdout;
    void* redirect_fp;  // FILE* but kept as void* to avoid including stdio.h
} myshell_redirect_state_t;

// Setup output redirection (> or >>)
// Returns redirect state on success, or state with saved_stdout=-1 on failure
myshell_redirect_state_t myshell_setup_output_redirection(const char* filename, bool append);

// Restore stdout to original state
void myshell_restore_output_redirection(myshell_redirect_state_t* state);

#endif // OUTPUT_REDIRECTION_H
