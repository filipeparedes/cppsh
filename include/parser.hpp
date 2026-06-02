/**
 * @file parser.hpp
 * @brief Defines the command struct and the Parser class, containing all methods related to command parsing.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.2.0
 * @date 2026-04-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <vector>
#include <string>

#include "command.hpp"

namespace cppsh {
    /**
     * @class Parser
     * @brief Parses a raw input string into Command struct.
     */
    class Parser {
        public: 
            /**
             * @brief Parses a raw input line into a Command.
             * 
             * @param input The raw input string from the user.
             * @return A Command struct with args and redirections populated.
             */
            cppsh::Command parse(const std::string& input) const;

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
             * @param tok_vec [in, out] The user input as a vector of string tokens.
             * @param cmd [in, out] The command object.
             */
            void Parser::redirect_io(std::vector<std::string>& tok_vec, Command& cmd) const;
    };
}
