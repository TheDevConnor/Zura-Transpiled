#pragma once

#include "../../../inc/colorize.hpp"
#include "../../lexer/lexer.hpp"
#include "../../ast/ast.hpp"
#include "../../common.hpp"
#include <iostream>
#include <cstring>

class Error {
public:
  static void error(Lexer::Token token, std::string msg, Lexer &lexer) {
    int errorCount = 0;
    std::cout << termcolor::yellow << "Error" << termcolor::reset
              << ": [line: " << termcolor::blue << token.line << termcolor::reset
              << ", column: " << termcolor::blue << token.column - 1
              << termcolor::reset << "] " << termcolor::red << msg
              << termcolor::reset;

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

  // static void errorType(AstNode::Type *type, AstNode::Type *returnType,
  //                         std::string name) {
  //   if (type == nullptr || returnType == nullptr || type->type.start == nullptr || returnType->type.start == nullptr) {
  //     std::cerr << termcolor::red << "Error: " << termcolor::reset << "Null pointer encountered in function '" 
  //               << termcolor::yellow << name << termcolor::reset << "'" << std::endl; 
  //     Exit(ExitValue::INVALID_TYPE);
  //   }

  //   if (strcmp(type->type.start, returnType->type.start) == 0) {
  //     return;
  //   }

  //   std::cout << termcolor::red << "Error: " << termcolor::reset
  //             << "Expected type '" << termcolor::green << type->type.start << termcolor::reset 
  //             << "' but got '" << termcolor::green << returnType->type.start <<termcolor::reset
  //             << "' in function '" << termcolor::yellow << name << termcolor::reset << "'" 
  //             << std::endl;
  // }
};
