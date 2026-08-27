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
 * @version 1.6.0
 * @date 2026-08-28 
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <string>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#include <expected>
#include <print>
#include <optional>

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
std::string get_prompt(const std::string& user, const std::string& hostname) {
    return std::format("{}@{}:{}$ ", user, hostname, get_cwd());
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
    handle_signal();

    while(true) {
        std::string prompt = get_prompt(get_username(), get_hostname());

        std::optional<std::string> input_opt = read_input(prompt);

        //EOF (CTRL+D) - exit gracefully
        if (!input_opt) {
            std::println("");
            break;
        }

        std::string input = std::move(input_opt.value());

        //Ignore blank lines
        if (input.empty() || input.find_first_not_of(" \t") == std::string::npos) continue;

        //add to history
        auto add = add_to_history(input);
        if (!add){
            set_exit_code(static_cast<int>(add.error().code));
            print(add.error());
        }

        //Parse input into Command-type obj
        auto par = parse(input, get_env_variables(), get_last_exit_code());
        if (!par) {
            set_exit_code(static_cast<int>(error_code_t::MISSING_REDIRECTION_TARGET));
            print(shell_error_t{error_code_t::MISSING_REDIRECTION_TARGET, "cppsh", "", par.error()});
            continue;
        }

        //Dispatch the command
        auto dis = dispatch(par.value());
        if (!dis) {
            set_exit_code(static_cast<int>(dis.error().code));
            print(dis.error());
        } else {
            set_exit_code(dis.value());
        }
    }

    return {};
}