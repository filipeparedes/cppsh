module;
/**
 * @file pipeline.cppm
 * @brief Defines the Pipeline struct and logical operators. 
 * A Pipeline is, at its core, a vector of Commands.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.1.0
 * @date 2026-08-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <vector>

export module cppsh.pipeline;

import cppsh.command;

export enum class logical_op_t {
    NONE,   // No logical operator
    AND,    // &&
    OR      // ||
};

export struct pipeline_t {
    std::vector<command_t> cmds;  // vector of the Commands
    bool bg = false;            // background execution flag
    logical_op_t op = logical_op_t::NONE; //Logical operator that comes next (e.g. cmd1 &&)
};

