/**
 * @file parse_exception.hpp
 * @brief Declaration for ParseExpection
 * 
 * @author Filipe Paredes (filipeparedes3@gmail.com)
 * 
 * @version 0.1
 * @date 2026-06-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <stdexcept>

namespace cppsh {

    class ParseException : public std::runtime_error {
        public:
            using std::runtime_error::runtime_error;
    };

}