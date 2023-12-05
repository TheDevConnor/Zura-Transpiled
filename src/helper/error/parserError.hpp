#include <iostream>

#include "../../../inc/colorize.hpp"
#include "../../lexer/lexer.hpp"

class ParserError {
public:
    static void error(Lexer::Token line, int pos, std::string msg) {
        Lexer lexer(line.start.c_str());
        // Parser Error: [line: 1, column: 1]
        std::cout << termcolor::yellow << "Parser Error" << termcolor::reset 
                  << ": [line: " << termcolor::blue << line.line << termcolor::reset
                  << ", column: " << termcolor::blue << pos << termcolor::reset << "] "  
                  << termcolor::red << msg << termcolor::reset << std::endl;
        
        // The line of the error
        const char* lineStart = lexer.lineStart(line.line);
        const char* lineEnd = lineStart;
        while (*lineEnd != '\n' && *lineEnd != '\0') lineEnd++;
        std::cout << termcolor::yellow << "  " << termcolor::reset << lineStart << std::endl;

        // The arrow pointing to the error
        int arrowPos = line.column;
        while (arrowPos > 1 && lineStart[arrowPos - 1] != '\n') arrowPos--;
        int arrowLength = lineEnd - lineStart;
        while (arrowLength > 0 && lineStart[arrowLength - 1] != '\n') arrowLength--;
        for (int i = 0; i < arrowPos; i++) std::cout << " ";
        for (int i = 0; i < arrowLength; i++) std::cout << "^";
        std::cout << std::endl;

        // The error message
        std::cout << termcolor::red << "  " << termcolor::reset << msg << std::endl;
    }
};