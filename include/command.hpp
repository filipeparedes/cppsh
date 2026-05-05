/**
 * @file command.hpp
 * @brief Defines the Command and CommandEntry structs.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.1
 * @date 2026-05-03
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
    //TODO: Redirection support
    //TODO: Background execution support
    struct Command {
    std::vector<std::string> args; //command name and arguments
    };

    /**
     * @brief Represents an entry in the command dispatch table.
     * 
     */
    struct CommandEntry {
    std::string name;                               //Command name (e.g. "exit", "cd")
    std::function<int(const Command&)> handler;     //Function to call when command is matched
    std::string description;                        // Human-readable description of the command for help
    };
}