#pragma once

#include <iostream>

#include "../../../inc/colorize.hpp"
#include "../../common.hpp"

class LexerError {
public:
    static void error(std::string message, int line, int column) {
        // Lexer Error: [line: 1, column: 1] <message>
        std::cerr << termcolor::yellow << "Lexer Error" << termcolor::reset 
                  << ": [line: " << termcolor::blue << line << termcolor::reset
                  << ", column: " << termcolor::blue << column << termcolor::reset << "] "  
                  << termcolor::red << message << termcolor::reset << std::endl;
        Exit(ExitValue::LEXER_ERROR);
    }
};