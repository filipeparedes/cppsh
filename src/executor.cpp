/**
 * @file executor.cpp
 * @brief Implementation for executor.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1.0
 * @date 2026-05-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "include/executor.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <iostream>
#include <csignal>

int Executor::execute(const cppsh::Command& cmd) {
    if (cmd.args.empty()) return 0;

    pid_t c_pid = fork();

    if (c_pid == -1) {
        perror("fork");
        return -1;
    }
    else if (c_pid > 0) {
        //Parent process
        int status;
        waitpid(c_pid, &status, WUNTRACED); // wait for child to end

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 0;
        } else if (WIFSTOPPED(status)) {
            return 0;
        }
        return 0;
    }
    else {
        //Child process
        setpgrp();
        signal(SIGINT, SIG_DFL); //Reset signal behaviour to default in child process
        signal(SIGTSTP, SIG_DFL);

        std::vector<char*> argv = cppsh::to_vchar(cmd.args);

        //.data() converts std::vector<char*> into char**
        execvp(cmd.args[0].c_str(), argv.data());

        //If the process reaches here, execvp failed
        exit(0);
    }
}