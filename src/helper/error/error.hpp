#pragma once

#include "../../../inc/colorize.hpp"
#include "../../ast/ast.hpp"
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

    const char *lineStart = lexer.lineStart(token.line);
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

  static void errorType(TypeAST *type, TypeAST *returnType, std::string name) {
    if (type == nullptr || returnType == nullptr ||
        type->Name.c_str() == nullptr || returnType->Name.c_str() == nullptr) {
      std::cerr << termcolor::red << "Error: " << termcolor::reset
                << "Null pointer encountered in function '" << termcolor::yellow
                << name << termcolor::reset << "'" << std::endl;
      Exit(ExitValue::INVALID_TYPE);
    }

    if (type->Name == returnType->Name)
      return;

    std::cout << termcolor::red << "Error: " << termcolor::reset
              << "Expected type '" << termcolor::green << type->Name
              << termcolor::reset << "' but got '" << termcolor::green
              << returnType->Name << termcolor::reset << "' on '"
              << termcolor::yellow << name << termcolor::reset << "'"
              << std::endl;
  }
};
