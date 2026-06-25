module;
/**
 * @file pipeline.cppm
 * @brief Defines the Pipeline struct. 
 * A Pipeline is, at its core, a vector of Commands.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.0.0
 * @date 2026-06-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <vector>

export module cppsh.pipeline;

import cppsh.command;

export struct pipeline_t {
    std::vector<command_t> cmds;  // vector of the Commands
    bool bg = false;            // background execution flag
};
