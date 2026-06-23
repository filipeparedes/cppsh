module;
/**
 * @file command.cppm
 * @brief Defines the Command struct.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 1.0.0
 * @date 2026-06-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <vector>
#include <string>

export module cppsh.command;

/**
 * @brief Represents a single parsed command.
*/
export struct command_t {
    std::vector<std::string> args;  // command name and arguments
    std::string input_file;         // from <
    std::string output_file;        // for > or >> 
    bool append = false;            // decide if > or >>
};