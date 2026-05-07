/**
 * @file parser.cpp
 * @brief Implementation for parser.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-04-30
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include "parser.hpp"
#include <iostream>
#include <sstream>

namespace cppsh {

    cppsh::Command Parser::parse(const std::string& input) const {
        std::vector<std::string> tokenized_str = tokenize(input);

        Command cmd;
        cmd.args = tokenized_str;

        return cmd;
    }

    std::vector<std::string> Parser::tokenize(const std::string& input) const {
        std::vector<std::string> tokens;
        std::stringstream stream(input);
        std::string token;

        //get each token from the strea
        while (stream >> token){
            tokens.push_back(token);
        }

        return tokens;
    }
}