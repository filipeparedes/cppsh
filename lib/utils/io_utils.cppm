module;
/**
 * @file str_utils.cppm
 * @brief Implementation of some utilitary I/O functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.0.1
 * @date 2026-08-31
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include <string>
#include <optional>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>

export module utils.io_utils;

export namespace io_utils {

    /**
     * @brief Reads a line of input from stdin.
     *
     * @return The line entered by the user, or std::nullopt on EOF.
     */
    std::optional<std::string> read_input(const std::string& prompt) {
        char* raw_input = readline(prompt.c_str());
        //raw_input null -> EOF (Ctrl+D was pressed)
        if (!raw_input) return std::nullopt;

        std::string line(raw_input);

        if (!line.empty()) {
            add_history(raw_input);
        }

        //readline allocates memory
        free(raw_input);
        return line;
    }

}