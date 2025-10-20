#define _POSIX_C_SOURCE 200809L  // Enable POSIX signal features

#include "main.h"
#include "log.h"
#include "myshell.h"  // For hash table pointer

int main(int argc, char* argv[]) {
    
    // Check global variable at startup
    
    // Parse command line arguments first
    myshell_parse_args(argc, argv);
    
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Starting MyShell with log level: %d", myshell_log_level);
    
    // Setup signal handlers
    myshell_setup_signal_handlers();
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Signal handlers configured");
    
    // Initialize terminal input
    myshell_init_term_input();
    MYSHELL_LOG(MYSHELL_LOG_LEVEL_DEBUG, "Terminal input initialized");
    
    // Show banner
    myshell_show_banner();
   
    // Enter main prompt loop
    myshell_do_prompt_loop();

    myshell_abort(0);
    return 0;
}