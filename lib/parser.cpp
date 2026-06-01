/**
 * @file parser.cpp
 * @brief Implementation for parser.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.2.0
 * @date 2026-06-01
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include "parser.hpp"
#include <iostream>
#include <sstream>

namespace cppsh {

    cppsh::Command Parser::parse(const std::string& input) const {
        std::vector<std::string> tok_vec = tokenize(input);
        Command cmd;

        // IO redirection
        for (int i = 0; i<tok_vec.size(); i++) {
            //Input Redirection
            if (tok_vec[i] == "<"){
                cmd.input_file = tok_vec[i + 1]; // next argument should be the file name

                tok_vec.erase(tok_vec.begin() + i); //erase redirection operator
                tok_vec.erase(tok_vec.begin() + i); //shifted down, erase file name

                i--;
            } 
            //Output Redirection (Append)
            else if (tok_vec[i] == ">>"){
                cmd.output_file = tok_vec[i + 1];
                cmd.append = true; // >> appends instead of overwriting

                tok_vec.erase(tok_vec.begin() + i);
                tok_vec.erase(tok_vec.begin() + i);

                i--;
            }
            //Output Redirection (Overwrite)
            else if (tok_vec[i] == ">"){
                cmd.output_file = tok_vec[i + 1];

                tok_vec.erase(tok_vec.begin() + i);
                tok_vec.erase(tok_vec.begin() + i);

                i--;
            }
        }
        cmd.args = tok_vec;

        return cmd;
    }

    std::vector<std::string> Parser::tokenize(const std::string& input) const {
        std::vector<std::string> tokens;
        std::stringstream stream(input);
        std::string token;

        //get each token from the stream
        while (stream >> token){
            tokens.push_back(token);
        }

        return tokens;
    }
}