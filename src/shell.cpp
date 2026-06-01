/**
 * @file shell.cpp
 * @brief Shell class implementation.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.5.0
 * @date 2026-04-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "shell.hpp"
#include "dispatcher.hpp"
#include "signal_handling.hpp"
#include "errors/shell_error.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#include <parse_exception.hpp>
#include <parser.hpp>
#include <utils.hpp>

Shell::Shell() : context(dsptchr){
    m_hostname = cppsh::get_hostname();
    m_user = cppsh::get_username();
    handle_signal();
}

void Shell::run() {
    std::string input;
    cppsh::Command cmd;

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

        context.history.push_back(input);

        try {
            //Parse input into Command-type obj
            cmd = parser.parse(input);

            dsptchr.dispatch(cmd, context);
        }
        catch (const cppsh::ParseException& e) {
            ShellError(ShellErrorCode::MISSING_REDIRECTION_TARGET, "cppsh", "", e.what()).print();
        }
        catch (ShellError& e) {
            e.print();
        }

    }
}

void Shell::print_prompt() const {
    std::cout << m_user << "@" << m_hostname << ":" << cppsh::get_cwd() << "$ " << std::flush;
}