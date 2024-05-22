#pragma once

#include "../../../inc/colorize.hpp"
#include "../../common.hpp"
#include "../../lexer/lexer.hpp"
#include <iostream>

class ErrorClass {
public:
  static void error(Lexer::Token token, std::string msg, Lexer &lexer) {
    int errorCount = 0;
    std::cout << termcolor::yellow << "Error" << termcolor::reset
              << ": [line: " << termcolor::blue << token.line
              << termcolor::reset << ", column: " << termcolor::blue
              << token.column - 1 << termcolor::reset << "] " << termcolor::red
              << msg << termcolor::reset;

    const char *lineStart = lexer.lineStart(token.line, lexer.scanner.source);
    const char *lineEnd = lineStart;
    while (*lineEnd != '\n' && *lineEnd != '\0')
      lineEnd++;
    std::cout << std::endl
              << std::string(lineStart, lineEnd - lineStart) << std::endl;

    std::string length(lineStart, lineEnd - lineStart);
    std::cout << termcolor::green << std::string(token.column - 2, '~')
              << termcolor::red << "^" << termcolor::reset << std::endl;

    if (errorCount == 5)
      Exit(ExitValue::_ERROR);
    errorCount++;
  }
};
