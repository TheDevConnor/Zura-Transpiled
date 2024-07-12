#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "../../lexer/lexer.hpp"

namespace ErrorClass {
  inline static std::unordered_map<int, std::string> errors;
  std::string error(int line, int pos, std::string msg, std::string errorType, std::string filename, 
                    Lexer &lexer, std::vector<Lexer::Token> tokens, bool isParser = false,
                    bool isWarning = false, bool isNote = false, bool isFatal = false);
  void printError();
  std::string currentLine(int line, int pos, Lexer &lexer, bool isParser, std::vector<Lexer::Token> tokens);

  std::string lineNumber(int line);
  std::string printLine(int line, const char *start);
};
