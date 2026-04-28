/**
 * @file shell.cpp
 * @brief Shell class implementation.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-04-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "shell.hpp"

#include <iostream>
#include <string>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>

Shell::Shell() {
    //Resolve username from the current user ID
    struct passwd* pw = getpwuid(getuid());
    m_user = pw ? pw->pw_name : "user";

    //Resolve hostname
    char hostname[_POSIX_HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == 0){
        m_hostname = hostname;
    } else {
        m_hostname = "localhost";
    }
}

void Shell::run() {
    std::string input;

    while(true) {
        print_prompt();

        input = read_input();

        //EOF (CTRL+D) - exit gracefully
        if (input.empty()) {
            std::cout << std::endl;
            break;
        }

        //Ignore blank lines
        if (input.find_first_not_of("/t") == std::string::npos) continue;

        //TODO: parse and execute input
        std::cout << "Input: " << input << std::endl;
    }
}

void Shell::print_prompt() const {
    std::cout << m_user << "@" << m_hostname << ":" << get_cwd() << "$ " << std::flush;
}

std::string Shell::read_input() const {
    std::string line;

    if (!std::getline(std::cin, line)) return "";

    return line;
}

std::string Shell::get_cwd() const {
    char buffer[PATH_MAX];

    if (getcwd(buffer, sizeof(buffer)) == nullptr) {
        return "?";
    }

    std::string cwd(buffer);

    //Replace home directory prefix with ~
    const char* home = getenv("HOME");
    if (home) {
        std::string home_str(home);
        if (cwd.find(home_str) == 0) {
            cwd = "~" + cwd.substr(home_str.size());
        }
    }

    return cwd;
}