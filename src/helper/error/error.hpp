#pragma once

#include <string>

#include "../../lexer/lexer.hpp"

class ErrorClass {
public:
  static void error(int line, int pos, std::string msg, std::string errorType, std::string filename);
private: 
  static void beforeLine(int line);
  static void currentLine(int line, int pos);
  static void afterLine(int line);

  static const char *lineNumber(int line);
  static void printLine(int line, const char *start);
};
