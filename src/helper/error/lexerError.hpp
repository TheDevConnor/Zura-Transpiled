#pragma once

#include <iostream>

#include "../../../inc/colorize.hpp"
#include "../../common.hpp"
#include "../../lexer/lexer.hpp"

class LexerError {
public:
  static void error(std::string message, int line, int column) {
    // Lexer Error: [line: 1, column: 1] <message>
    std::cerr << termcolor::yellow << "Error" << termcolor::reset
              << ": [line: " << termcolor::blue << line << termcolor::reset
              << ", column: " << termcolor::blue << column << termcolor::reset
              << "] " << termcolor::red << message << termcolor::reset;

    const char *lineStart = lexer.lineStart(line);
    const char *lineEnd = lineStart;
    while (*lineEnd != '\n' && *lineEnd != '\0')
      lineEnd++;
    std::cout << std::endl
              << std::string(lineStart, lineEnd - lineStart) << std::endl;

    std::string length(lineStart, lineEnd - lineStart);
    std::cout << termcolor::green << std::string(column - 2, '~')
              << termcolor::red << "^" << termcolor::reset << std::endl;

    Exit(ExitValue::LEXER_ERROR);
  }
};
