/**
 * @file command.hpp
 * @brief Defines the Command struct.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.3.0
 * @date 2026-05-31
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <vector>
#include <string>
#include <functional>

namespace cppsh {

    /**
     * @brief Represents a single parsed command.
     */
    //TODO: Background execution support
    struct Command {
        std::vector<std::string> args;  // command name and arguments
        std::string input_file;         // from <
        std::string output_file;        // for > or >> 
        bool append = false;            // decide if > or >>
    };
}