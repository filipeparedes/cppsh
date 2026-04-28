/**
 * @file shell.hpp
 * @brief Main shell class declaration.
 * 
 * Defines the Shell class responsible for the main execution loop,
 * prompt rendering, parsing and execution.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-04-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 #pragma once

 #include <string>

 /**
  * @class Shell
  * @brief Represents the shell instance.
  * 
  * Manages the main read-eval-print loop (REPL), displays the prompt, reads user input
  * and delegates parsing and execution to the appropriate components.
  * 
  */
 class Shell {
    public: 
        /**
         * @brief Constructs a new Shell instance.
         */
        Shell();

        /**
         * @brief Starts the main shell loop.
         * 
         * Continuously displays the prompt, reads a line of input,
         * and processes it until the user exits.
         *
         */
        void run();

    private:
        std::string m_user; //Current logged in username
        std::string m_hostname; // Machine hostname

        /**
         * @brief Builds and prints the shell prompt.
         * 
         * Displays the prompt in the format: user@hostname:~/path$
         * 
         */
        void print_prompt() const;

        /**
         * @brief Reads a line of input from stdin.
         * 
         * @return The line entered by the user, or an empty string on EOF.
         */
        std::string read_input() const;

        /**
         * @brief Resolves the current working directory.
         * 
         * Replaces the home directory prefix with - if applicable.
         * 
         * @return A human-readable representation of the current directory.
         */
        std::string get_cwd() const;
 };