#pragma once

#include "../../lexer/lexer.hpp"
#include "../../ast/ast.hpp"

#include <unordered_map>
#include <vector>
#include <string>

namespace ErrorClass {
  inline static std::unordered_map<int, std::string> errors;
  inline static std::vector<std::string> typeErros;
  std::string error(int line, int pos, std::string msg, std::string note, std::string errorType, std::string filename, 
                    Lexer &lexer, std::vector<Lexer::Token> tokens, bool isParser = false,
                    bool isWarning = false, bool isFatal = false, bool isMain = false, bool isTypeError = false);

  void printError();
  std::string currentLine(int line, int pos, Lexer &lexer, bool isParser, std::vector<Lexer::Token> tokens);

  std::string lineNumber(int line);
  std::string printLine(int line, const char *start);
};
