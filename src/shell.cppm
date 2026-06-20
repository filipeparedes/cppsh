module;
/**
 * @file shell.cppm
 * @brief Main shell functions
 * 
 * The Shell functions are responsible for the main execution loop,
 * prompt rendering, parsing and execution.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.0.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <iostream>
#include <string>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#include <expected>

export module cppsh.shell;

import cppsh.dispatching;
import cppsh.parsing;
import cppsh.signal_handling;
import cppsh.shell_errors;
import cppsh.utils;
import cppsh.shell_state;
import cppsh.pipeline;


/**
 * @brief Builds and prints the shell prompt.
 * 
 * Displays the prompt in the format: user@hostname:~/path$
 * 
 */
void print_prompt(const std::string& user, const std::string& hostname) {
    std::cout << user << "@" << hostname << ":" << get_cwd() << "$ " << std::flush;
}

/**
 * @brief Starts the main shell loop.
 * 
 * Continuously displays the prompt, reads a line of input,
 * and processes it until the user exits.
 *
 * @returns Unexpected: shell_error_t
 */
export std::expected<void, shell_error_t> run() {
    std::string hostname = get_hostname();
    std::string user = get_username();
    shell_state_t state;

    handle_signal();

    while(true) {
        print_prompt(user, hostname);

        std::string input = read_input();

        //EOF (CTRL+D) - exit gracefully
        if (input.empty()) {
            std::cout << std::endl;
            break;
        }

        //Ignore blank lines
        if (input.find_first_not_of(" \t") == std::string::npos) continue;

        state.history.push_back(input);

        //Parse input into Command-type obj
        std::expected<pipeline_t, std::string> par = parse(input);
        if (!par) {
            print(shell_error_t{error_code_t::MISSING_REDIRECTION_TARGET, "cppsh", "", par.error()});
            continue;
        }

        std::expected<int, shell_error_t> dis = dispatch(par.value(), state);
        if (!dis)
            print(dis.error());
    }

    return {};
}