module;
/**
 * @file parsing.cppm
 * @brief Implementation for parsing related functions
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 2.1.0
 * @date 2026-08-31
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
 * @brief Expands a POSIX environment variable into its value.
 * 
 * @param input The raw input line.
 * @param i Current index in the input (pointing at '$'). Updated to the last char of the var name
 * @param env_vars Map containing environment variables.
 *
 * @return The expanded value, or an empty string if variable is not set.
 */
std::string expand_variable(const std::string& input, size_t& i,
                            const std::unordered_map<std::string, env_entry_t>& env_vars) {
    std::string var_name;
    size_t next_i = i+1; //start reading after $

    //only valid chars for POSIX identifiers
    //e.g "$DRV_A/docs" -> stops immediately at '/'
    while (next_i < input.size() && (std::isalnum(static_cast<unsigned char>(input[next_i])) || input[next_i] == '_')) {
        var_name += input[next_i];
        next_i++;
    }

    //handle isolated $ as a literal
    if (var_name.empty()) return "";

    //advance caller index to the end of the variable name
    i = next_i-1;
    
    auto it = env_vars.find(var_name);
    return (it != env_vars.end()) ? it->second.value : "";
}

/**
 * @brief Splits the input string into tokens.
 *    Accounts for quotes, and escape characters.
 *    Reads the input char by char.
 *    Ignores '\' at the end of the string.
 *    Unclosed quotes '"' or "'" will treat everything as a literal until EOF
 * 
 * @param input The raw input string.
 * @param env_vars An unordered map of the environment variables
 * @param last_exit_code The exit code for the last executed command
 *
 * @return A vector of string tokens.
 */
std::vector<std::string> tokenize(
    const std::string& input, 
    const std::unordered_map<std::string, env_entry_t>& env_vars,
    int last_exit_code
) {
    std::vector<std::string> tokens;
    std::string current;
    current.reserve(32); //avoid heap reallocations

    bool is_escaped = false;
    Quote quote = Quote::None;

    //loop the string char by char
    for (size_t i=0; i<input.size(); ++i){
        char c = input[i];

        //Escaped characters
        if (is_escaped) {
            current += c;
            is_escaped = false;
            continue;
        }
        if (c == '\\' && quote != Quote::Single){
            is_escaped = true;
            continue;
        }

        //Quote state toggle
        if (c == '"' && quote != Quote::Single){
            quote = (quote == Quote::Double) ? Quote::None : Quote::Double;
            continue;
        }
        if (c == '\'' && quote != Quote::Double) {
            quote = (quote == Quote::Single) ? Quote::None : Quote::Single;
            continue;
        }
        
        //Variable expansion ($VAR)
        //only if quotes not single ('')
        if (c == '$' && quote != Quote::Single) {
            //$? => return last exit code
            if ((i+1)<input.size() && input[i+1] == '?'){
                current.append(std::to_string(last_exit_code));
                i++;
            } else {
                size_t old_i = i;
                std::string val = expand_variable(input, i, env_vars);

                if (i != old_i) //Var name was found and index advanced
                    current.append(val);
            }   
            continue;
        }

        //Word boundary (spaces outside quotes)
        if(std::isspace(static_cast<unsigned char>(c)) && quote == Quote::None){
            //ignore empty tokens, like ""
            if(!current.empty()){
                tokens.push_back(std::move(current));
                current.clear();
            }
        }
        else {
            current += c;
        }
    }

    //add last token
    if (!current.empty())
        tokens.push_back(std::move(current));

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
 * @brief Splits the tokens into a list of pipelines (divided by && and ||),
 * and splits each pipeline into commands (divided by |).
 * 
 * @param tok_vec [in, out] The user input as a vector of string tokens.
 * @param log_pl [in, out] The vector of pipelines.
 */
void split(std::vector<std::string>& tok_vec, std::vector<pipeline_t>& log_pl) {
    pipeline_t pl;

    for (int i=0; i<tok_vec.size(); i++) {
        //Look for pipe symbol or logical operator
        if (tok_vec[i] == "|" || tok_vec[i] == "&&" || tok_vec[i] == "||"){
            command_t cmd;

            //Create command with the tokens
            //there shouldn't exist any other pipe symbols before the pipe in tok_vec[i]
            if (i > 0) {
                cmd.args = std::vector<std::string>(tok_vec.begin(), tok_vec.begin() + i);
                pl.cmds.push_back(cmd);
            }

            //If logical operator, close pipeline and store it
            if (tok_vec[i] == "&&" || tok_vec[i] == "||" ){
                pl.op = (tok_vec[i] == "&&") ? logical_op_t::AND : logical_op_t::OR;
                log_pl.push_back(std::move(pl));
                pl = pipeline_t{}; //reset pipeline
            }

            //delete the command portion and operator
            tok_vec.erase(tok_vec.begin(), tok_vec.begin() + i+1);

            //set to -1 => loop's i++ changes back to 0
            i = -1;
        }
    }

    //add last token if exists
    if (!tok_vec.empty()) {
        command_t cmd;
        cmd.args = tok_vec;
        pl.cmds.push_back(cmd);
    }

    //add last pipeline if exists
    if (!pl.cmds.empty()) {
        log_pl.push_back(std::move(pl));
    }
}

/**
 * @brief Checks for background execution instruction in the user input
 * 
 * Writes to pl if it is found.
 * Also removes that instruction from tok_vec
 * 
 * @param tok_vec [in, out] The user input as a vector of string tokens.
 * @param is_background [out] Boolean flag to set.
 */
void is_bg(std::vector<std::string>& tok_vec, bool& is_background) {
    if (!tok_vec.empty() && tok_vec.back() == "&") {
        is_background = true;
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
 * @brief Parses a raw input line into a list of Pipelines.
 * 
 * @param input The raw input string from the user.
 * @param env_vars An unordered map of the environment variables
 * @param last_exit_code The exit code for the last executed command
 *
 * @return expected: A vector of pipeline structs with populated Commands.
 * @return unexpected: A string with the error message
 */
export std::expected<std::vector<pipeline_t>, std::string> parse(
    const std::string& input,
    const std::unordered_map<std::string, env_entry_t>& env_vars = {},
    int last_exit_code = 0
){
    std::vector<std::string> tok_vec = tokenize(input, env_vars, last_exit_code);
    std::vector<pipeline_t> log_pl;

    bool is_background = false;
    is_bg(tok_vec, is_background);

    split(tok_vec, log_pl);

    //Background is applied to the last pipeline
    if (!log_pl.empty()){
        log_pl.back().bg = is_background;
    }

    for(pipeline_t& pl : log_pl) {
        for (command_t& cmd : pl.cmds) {
            std::expected<bool, std::string> assign_res = is_assignment(cmd);
            if (!assign_res) return std::unexpected(assign_res.error());
            if (assign_res.value()) cmd.type = command_type_t::assignment;

            std::expected<void, std::string> redi_res = redirect_io(cmd);
            if (!redi_res) return std::unexpected(redi_res.error()); 
        }  
    }
    return log_pl;
}
