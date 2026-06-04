/**
 * @file parser.hpp
 * @brief Defines the command struct and the Parser class, containing all methods related to command parsing.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.3.0
 * @date 2026-04-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <vector>
#include <string>

#include "pipeline.hpp"
#include "command.hpp"

namespace cppsh {
    /**
     * @class Parser
     * @brief Parses a raw input string into Pipeline struct.
     */
    class Parser {
        public: 
            /**
             * @brief Parses a raw input line into a Pipeline.
             * 
             * @param input The raw input string from the user.
             * @return A Pipeline struct with populated Commands.
             */
            cppsh::Pipeline parse(const std::string& input) const;

        private:
            /**
             * @brief Splits the input string into tokens by whitespace.
             * 
             * @param input The raw input string.
             * @return A vector of string tokens.
             */
            std::vector<std::string> tokenize(const std::string& input) const;

            /**
             * @brief Checks for io redirection instructions in the user input
             * 
             * Writes to cmd if they are found.
             * Also removes those instructions from cmd.args
             * 
             * @param cmd [in, out] The command object.
             */
            void redirect_io(Command& cmd) const;

            /**
             * @brief Splits the tokens into a pipeline of commands
             * 
             * @param tok_vec [in, out] The user input as a vector of string tokens.
             * @param pl [in, out] The pipeline object
             */
            void split(std::vector<std::string>& tok_vec, Pipeline& pl) const;

            /**
             * @brief Checks for background execution instruction in the user input
             * 
             * Writes to pl if it is found.
             * Also removes that instruction from tok_vec
             * 
             * @param tok_vec [in, out] The user input as a vector of string tokens.
             * @param pl [in, out] The Pipeline object.
             */
            void inline is_bg(std::vector<std::string>& tok_vec, Pipeline& pl) const {
                if (!tok_vec.empty() && tok_vec.back() == "&") {
                    pl.bg = true;
                    tok_vec.pop_back();
                }
            }
    };
}
