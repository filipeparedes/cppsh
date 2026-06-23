/**
 * @file main.cpp
 * @brief Entry point for cppsh.
 * 
 * Starts the main shell loop.
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * @version 0.2.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 import cppsh.shell;

 int main() {
   auto res = run();
   if (!res)
      return 1;
   return 0;
 }