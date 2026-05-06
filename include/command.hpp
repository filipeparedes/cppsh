/**
 * @file command.hpp
 * @brief Defines the Command and CommandEntry structs.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.2.0
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
}