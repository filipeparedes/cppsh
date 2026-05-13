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
        waitpid(c_pid, &status, 0); // wait for child to end
        return WEXITSTATUS(status);
    }
    else {
        //Child process
        std::vector<char*> argv = cppsh::to_vchar(cmd.args);

        //.data() converts std::vector<char*> into char**
        execvp(cmd.args[0].c_str(), argv.data());

        //If the process reaches here, execvp failed
        exit(0);
    }
}