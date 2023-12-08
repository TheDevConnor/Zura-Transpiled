#include <iostream>

#include "../../../inc/colorize.hpp"
#include "../../lexer/lexer.hpp"

class ParserError {
public:
    static void error(Lexer::Token line, int pos, std::string msg, Lexer& lexer) {
        // Parser Error: [line: 1, column: 1]
        std::cout << termcolor::yellow << "Parser Error" << termcolor::reset 
                  << ": [line: " << termcolor::blue << line.line << termcolor::reset
                  << ", column: " << termcolor::blue << pos << termcolor::reset << "] "  
                  << termcolor::red << msg << termcolor::reset;
        
        // The line of the error
        const char* lineStart = line.start - line.column;
        const char* lineEnd = lineStart + 1;
        while (*lineEnd != '\n' && *lineEnd != '\0') lineEnd++;
        std::cout << termcolor::yellow << "  " << termcolor::reset << lineStart << std::endl;

        // The arrow pointing to the error
        int arrowPos = line.column - 1;
        while (lineStart != lineEnd) {
            if (*lineStart == '\t') {
                std::cout << termcolor::yellow << " " << termcolor::reset << '\t';
                arrowPos += 3;
            } else {
                std::cout << termcolor::yellow << "^" << termcolor::reset;
                arrowPos += 0.5;
            }
            lineStart++;
        }
        for (int i = 0; i < arrowPos - 2; i++) std::cout << ' ';
        std::cout << termcolor::red << '^' << termcolor::reset << std::endl;

        // The error message
        std::cout << termcolor::red << "  " << termcolor::reset << msg << std::endl;
    }
};