module;
/**
 * @file env_entry.hpp
 * @brief Defines the env entry struct, containing the details for a single
 *      environment variable entry.
 *
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 *
 * @version 0.0.1
 * @date 2026-08-12
 *
 * @copyright Copyright (c) 2026
 *
 */

 #include <string>

 export module cppsh.env_entry;

 /**
  * @brief Holds the details for a single environment variable
  */
 export struct env_entry_t {
    std::string value;
    bool is_exported = false; 
 };