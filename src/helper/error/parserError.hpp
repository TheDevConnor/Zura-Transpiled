#include <iostream>

#include "../../../inc/colorize.hpp"
#include "../../common.hpp"
#include "../../lexer/lexer.hpp"

class ParserError {
public:
  static void error(Lexer::Token line, std::string msg, Lexer &lexer) {
    // Parser Error: [line: 1, column: 1]
    std::cout << termcolor::yellow << "Error" << termcolor::reset
              << ": [line: " << termcolor::blue << line.line << termcolor::reset
              << ", column: " << termcolor::blue << line.column - 1
              << termcolor::reset << "] " << termcolor::red << msg
              << termcolor::reset;

    const char *lineStart = lexer.lineStart(line.line);
    const char *lineEnd = lineStart;
    while (*lineEnd != '\n' && *lineEnd != '\0')
      lineEnd++;
    std::cout << std::endl
              << std::string(lineStart, lineEnd - lineStart) << std::endl;

    std::string length(lineStart, lineEnd - lineStart);
    std::cout << termcolor::green << std::string(line.column - 2, '~')
              << termcolor::red << "^" << termcolor::reset << std::endl;

    Exit(ExitValue::PARSER_ERROR);
  }
};
