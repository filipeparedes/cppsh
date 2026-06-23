module;
/**
 * @file signal_handling.cppm
 * @brief Implementation for signal handling functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 1.0.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <csignal>
#include <unistd.h>

export module cppsh.signal_handling;

/**
  * @brief Private function
  * 
  * Handles SIGINT, which is triggered when pressing Ctrl+C
  * 
  * The child process is immediately terminated.
  * 
  * @param signum The signal number
  */
void handle_sigint(int signum) {
    signal(SIGINT, SIG_IGN); //parent process ignores signal
    write(STDOUT_FILENO, "\r\033[K", 4); //hide ^C
}


/**
 * @brief Private function
 * 
 * Handles SIGTSTP, which is triggered when pressing Ctrl+Z
 * 
 * The child process is suspended and sent to background.
 * 
 * @param signum The signal number
 */
void handle_sigtstp(int signum) {
    signal(SIGTSTP, SIG_IGN); //parent process ignores signal
    write(STDOUT_FILENO, "\r\033[K", 4); //hide ^Z
}

/**
 * @brief Function to handle all signals.
 */
export void handle_signal() {
    struct sigaction sa_int;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, nullptr);

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, nullptr);
}
