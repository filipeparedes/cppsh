/**
 * @file signal_handling.cpp
 * @brief Implementation for signal_handling.hpp, as well as some private functions for specific signals
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.1
 * @date 2026-05-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "include/signal_handling.hpp"

#include <csignal>
#include <unistd.h>

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

void handle_signal() {
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
