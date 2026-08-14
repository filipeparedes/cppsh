module;
/**
 * @file parsing.cppm
 * @brief Implementation for parsing related functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 1.5.0
 * @date 2026-08-14
 * 
 * @copyright Copyright (c) 2026
 * 
*/

#include <cctype>
#include <unordered_map>
#include <vector>
#include <string>
#include <expected>

export module cppsh.parsing;

import cppsh.pipeline;
import cppsh.command;
import cppsh.env_entry;

enum class Quote {
    None,
    Double,
    Single
};

/**
 * @brief Splits the input string into tokens.
 *    Accounts for quotes, and escape characters.
 *    Reads the input char by char.
 *    Ignores '\' at the end of the string.
 *    Unclosed quotes '"' or "'" will treat everything as a literal until EOF
 * 
 * @param input The raw input string.
 * @param env_vars An unordered map of the environment variables
 *
 * @return A vector of string tokens.
 */
std::vector<std::string> tokenize(const std::string& input, const std::unordered_map<std::string, env_entry_t>& env_vars) {
    std::vector<std::string> tokens;
    bool is_escaped = false;
    Quote quote = Quote::None;
    std::string current;

    //loop the string char by char
    for (size_t i=0; i<input.size(); ++i){
        char c = input[i];

        //quotes & escape characters handling
        if (is_escaped) {
            current += c;
            is_escaped = false;
            continue;
        }
        if (c == '\\' && quote != Quote::Single){
            is_escaped = true;
            continue;
        }
        if (c == '"' && quote == Quote::None) {
            quote = Quote::Double;
            continue;
        }
        if (c == '"' && quote == Quote::Double) {
            quote = Quote::None;
            continue;
        }
        if (c == '\'' && quote == Quote::None) {
            quote = Quote::Single;
            continue;
        }
        if (c == '\'' && quote == Quote::Single) {
            quote = Quote::None;
            continue;
        }
        
        //var expansion: only if quotes not single ('')
        if (c == '$' && quote != Quote::Single) {
            std::string var_name;
            size_t next_i = i+1; //start reading after $

            //only valid chars for POSIX identifiers
            //e.g "$DRV_A/docs" -> stops immediately at '/'
            while (next_i < input.size() && (std::isalnum(static_cast<unsigned char>(input[next_i])) || input[next_i] == '_')) {
                var_name += input[next_i];
                next_i++;
            }

            //found a valid name after '$'
            if (!var_name.empty()) {
                auto it = env_vars.find(var_name);
                if (it != env_vars.end()) {
                    current += it->second.value; //get respective value
                }
                //continue main loop until end of var name
                i = next_i-1;
                continue;
            }
            //treat isolated $ as literal
        }

        if(std::isspace(static_cast<unsigned char>(c)) && quote == Quote::None){
            //ignore empty tokens, like ""
            if(!current.empty()){
                tokens.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    //add last token
    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

/**
 * @brief Checks for io redirection instructions in the user input
 * 
 * Writes to cmd if they are found.
 * Also removes those instructions from cmd.args
 * 
 * @param cmd [in, out] The command object.
 */
std::expected<void, std::string> redirect_io(command_t& cmd) {
    // IO redirection
    for (int i = 0; i<cmd.args.size(); i++) {
        //Input Redirection
        if (cmd.args[i] == "<"){
            if (i + 1 >= cmd.args.size())
                return std::unexpected("missing redirection target after '<'");

            cmd.input_file = cmd.args[i + 1]; // next argument should be the file name

            cmd.args.erase(cmd.args.begin() + i); //erase redirection operator
            cmd.args.erase(cmd.args.begin() + i); //shifted down, erase file name

            i--;
        } 
        //Output Redirection (Append)
        else if (cmd.args[i] == ">>"){
            if (i + 1 >= cmd.args.size())
                return std::unexpected("missing redirection target after '>>'");

            cmd.output_file = cmd.args[i + 1];
            cmd.append = true; // >> appends instead of overwriting

            cmd.args.erase(cmd.args.begin() + i);
            cmd.args.erase(cmd.args.begin() + i);

            i--;
        }
        //Output Redirection (Overwrite)
        else if (cmd.args[i] == ">"){
            if (i + 1 >= cmd.args.size())
                return std::unexpected("missing redirection target after '>'");

            cmd.output_file = cmd.args[i + 1];

            cmd.args.erase(cmd.args.begin() + i);
            cmd.args.erase(cmd.args.begin() + i);

            i--;
        }
    }
    return {};
}

/**
 * @brief Splits the tokens into a pipeline of commands
 * 
 * @param tok_vec [in, out] The user input as a vector of string tokens.
 * @param pl [in, out] The pipeline_t data
 */
void split(std::vector<std::string>& tok_vec, pipeline_t& pl) {
    for (int i=0; i<tok_vec.size(); i++) {
        //Look for pipe symbol
        if (tok_vec[i] == "|"){
            command_t cmd;

            //copy every token until the '|'
            //there shouldn't exist any other pipe symbols before the pipe in tok_vec[i]
            cmd.args = std::vector<std::string>(tok_vec.begin(), tok_vec.begin() + i);
            pl.cmds.push_back(cmd);

            //delete the command portion
            tok_vec.erase(tok_vec.begin(), tok_vec.begin() + i+1);

            //set to -1 so for loop's i++ changes it to 0
            i = -1;
        }
    }

    //add last token if exists
    if (!tok_vec.empty()) {
        command_t cmd;
        cmd.args = tok_vec;
        pl.cmds.push_back(cmd);
    }
}

/**
 * @brief Checks for background execution instruction in the user input
 * 
 * Writes to pl if it is found.
 * Also removes that instruction from tok_vec
 * 
 * @param tok_vec [in, out] The user input as a vector of string tokens.
 * @param pl [in, out] The pipeline_t data.
 */
void is_bg(std::vector<std::string>& tok_vec, pipeline_t& pl) {
    if (!tok_vec.empty() && tok_vec.back() == "&") {
        pl.bg = true;
        tok_vec.pop_back();
    }
}

/**
 * @brief Checks for assignment type command
 * 
 * @param cmd The command to analyze
 *
 * @return expected: true if is assignment, false it if isn't
 * @return unexpected: string with error message
 */
std::expected<bool, std::string> is_assignment(const command_t& cmd) {
    if (cmd.args.empty()) return false;

    const std::string& arg = cmd.args[0];
    size_t eq_pos = arg.find('=');

    //NO '=' means not assignment => regular command
    if (eq_pos == std::string::npos) return false;

    //Has '=' => assignment
    std::string key = arg.substr(0, eq_pos);

    if (key.empty() || (!std::isalpha(key[0]) && key[0] != '_')) {
        return std::unexpected("var name must start with a letter or underscore");
    }

    for (char c : key) {
        if (!std::isalnum(c) && c != '_') {
            return std::unexpected("var name must be alphanumeric");
        }
    }

    return true;
}

/**
 * @brief Parses a raw input line into a Pipeline.
 * 
 * @param input The raw input string from the user.
 * @param env_vars An unordered map of the environment variables
 *
 * @return expected: A Pipeline struct with populated Commands.
 * @return unexpected: A string with the error message
 */
export std::expected<pipeline_t, std::string> parse(const std::string& input,
                                                    const std::unordered_map<std::string, env_entry_t>& env_vars) {

    std::vector<std::string> tok_vec = tokenize(input, env_vars);
    pipeline_t pl;

    is_bg(tok_vec, pl);
    split(tok_vec, pl);

    for(command_t& cmd : pl.cmds) {
        if (is_assignment(cmd)){
            cmd.type = command_type_t::assignment;
        }

        std::expected<void, std::string> res = redirect_io(cmd);
        if (!res) return std::unexpected(res.error());   
    }
    return pl;
}
