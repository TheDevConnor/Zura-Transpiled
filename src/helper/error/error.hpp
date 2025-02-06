#pragma once

#include "../../ast/ast.hpp"
#include "../../lexer/lexer.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace ErrorClass {
inline static std::unordered_map<int, std::string> errors = {};
inline int ErrorPos = 0;

std::string formatLineWithTokens(int line, int pos,
                                 const std::vector<Lexer::Token> &tokens,
                                 bool highlightPos, bool getLastToken = false);
std::string currentLine(int line, int pos, Lexer &lexer, bool isParser,
                        bool isTypeError,
                        const std::vector<Lexer::Token> &tokens, bool getLastToken = false);
std::string error(int line, int pos, const std::string &msg,
                  const std::string &note, const std::string &errorType,
                  const std::string &filename, Lexer &lexer,
                  const std::vector<Lexer::Token> &tokens, bool isParser,
                  bool isWarning, bool isFatal, bool isMain, bool isTypeError, bool isGeneration);

bool printError();

std::string lineNumber(int line);
std::string printLine(int line, const char *start);
}; // namespace ErrorClass
