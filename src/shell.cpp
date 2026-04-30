/**
 * @file shell.cpp
 * @brief Shell class implementation.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.2.0
 * @date 2026-04-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "shell.hpp"
#include "utils.hpp"
#include "parser.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>

Shell::Shell() {
    m_hostname = cppsh::get_hostname();
    m_user = cppsh::get_username();
}

void Shell::run() {
    std::string input;

    while(true) {
        print_prompt();

        input = cppsh::read_input();

        //EOF (CTRL+D) - exit gracefully
        if (input.empty()) {
            std::cout << std::endl;
            break;
        }

        //Ignore blank lines
        if (input.find_first_not_of(" \t") == std::string::npos) continue;

        cppsh::Command cmd = parser.parse(input);
        //TODO: execute input
        std::cout << "Input: ";
        for(const std::string& arg : cmd.args){
            std::cout << arg << " ";
        }
        std::cout << std::endl;
    }
}

void Shell::print_prompt() const {
    std::cout << m_user << "@" << m_hostname << ":" << cppsh::get_cwd() << "$ " << std::flush;
}