/**
 * @file parser.cpp
 * @brief Implementation for parser.hpp
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.4.0
 * @date 2026-06-01
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include "parser.hpp"
#include "parse_exception.hpp"

#include <iostream>
#include <sstream>

namespace cppsh {

    cppsh::Pipeline Parser::parse(const std::string& input) const {
        std::vector<std::string> tok_vec = tokenize(input);
        Pipeline pl;

        is_bg(tok_vec, pl);
        split(tok_vec, pl);

        for(Command& cmd : pl.cmds) {
            redirect_io(cmd);   
        }
        return pl;
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

    void Parser::redirect_io(Command& cmd) const  {
        // IO redirection
        for (int i = 0; i<cmd.args.size(); i++) {
            //Input Redirection
            if (cmd.args[i] == "<"){
                if (i + 1 >= cmd.args.size())
                    throw cppsh::ParseException("missing redirection target after '<'");

                cmd.input_file = cmd.args[i + 1]; // next argument should be the file name

                cmd.args.erase(cmd.args.begin() + i); //erase redirection operator
                cmd.args.erase(cmd.args.begin() + i); //shifted down, erase file name

                i--;
            } 
            //Output Redirection (Append)
            else if (cmd.args[i] == ">>"){
                if (i + 1 >= cmd.args.size())
                    throw cppsh::ParseException("missing redirection target after '>>'");

                cmd.output_file = cmd.args[i + 1];
                cmd.append = true; // >> appends instead of overwriting

                cmd.args.erase(cmd.args.begin() + i);
                cmd.args.erase(cmd.args.begin() + i);

                i--;
            }
            //Output Redirection (Overwrite)
            else if (cmd.args[i] == ">"){
                if (i + 1 >= cmd.args.size())
                    throw cppsh::ParseException("missing redirection target after '>'");

                cmd.output_file = cmd.args[i + 1];

                cmd.args.erase(cmd.args.begin() + i);
                cmd.args.erase(cmd.args.begin() + i);

                i--;
            }
        }
    }

    void Parser::split(std::vector<std::string>& tok_vec, Pipeline& pl) const {
        for (int i=0; i<tok_vec.size(); i++) {
            //Look for pipe symbol
            if (tok_vec[i] == "|"){
                Command cmd;

                //copy every token until the '|'
                //there shouldn't exist any other pipe symbols before the pipe in tok_vec[i]
                cmd.args = std::vector<std::string>(tok_vec.begin(), tok_vec.begin() + i-1);
                pl.cmds.push_back(cmd);

                //delete the command portion
                tok_vec.erase(tok_vec.begin(), tok_vec.begin() + i);

                //reset back to counter
                i = 0;
            }
        }

        //add last token if exists
        if (!tok_vec.empty()) {
            Command cmd;
            cmd.args = tok_vec;
            pl.cmds.push_back(cmd);
        }
    }
}