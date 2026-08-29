/**
 * @file main.cpp
 * @brief Entry point for cppsh.
 * 
 * Starts the main shell loop.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 1.0.0
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 import cppsh.shell;
 import cppsh.scripting;

 int main(int argc, char* argv[]) {
    auto res = (argc == 2) ? execute_file(argv[1]) : run();

    if (!res)
      return 1;
    return 0;
 }