#pragma once

#include "../../ast/ast.hpp"
#include "../../lexer/lexer.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace ErrorClass {
inline static std::unordered_map<int, std::string> errors;
inline static std::vector<std::string> typeErros;

std::string formatLineWithTokens(int line, int pos,
                                 const std::vector<Lexer::Token> &tokens,
                                 bool highlightPos);
std::string currentLine(int line, int pos, Lexer &lexer, bool isParser,
                        bool isTypeError,
                        const std::vector<Lexer::Token> &tokens);
std::string error(int line, int pos, const std::string &msg,
                  const std::string &note, const std::string &errorType,
                  const std::string &filename, Lexer &lexer,
                  const std::vector<Lexer::Token> &tokens, bool isParser,
                  bool isWarning, bool isFatal, bool isMain, bool isTypeError);

void printError();

std::string lineNumber(int line);
std::string printLine(int line, const char *start);
}; // namespace ErrorClass
