/**
 * @file pipeline.hpp
 * @brief Defines the Pipeline struct. 
 * A Pipeline is, at its core, a vector of Commands.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-06-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "command.hpp"

#include <vector>
#include <string>
#include <functional>

namespace cppsh {

    struct Pipeline {
        std::vector<Command> cmds;  // vector of the Commands
        bool bg = false;            // background execution flag
    };
}