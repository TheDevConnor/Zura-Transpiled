#pragma once
#ifdef csk

#include <string>

#include "../../lexer/lexer.hpp"

class ErrorClass {
public:
  static void error(int line, int pos, std::string msg, std::string errorType, std::string filename, Lexer &lexer);
private: 
  static void beforeLine(int line, Lexer &lexer);
  static void currentLine(int line, int pos, Lexer &lexer);
  static void afterLine(int line, Lexer &lexer);

  static void printIgnoreSpaces(int line);
  static const char *lineNumber(int line);
  static void printLine(int line, const char *start);
};

#endif