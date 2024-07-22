#include <string>
#include <unordered_map>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "lexer.hpp"
#include "maps.hpp"

void Lexer::initLexer(const char *source) {
  scanner.current = source;
  scanner.source = source;
  scanner.start = source;
  scanner.column = 1;
  scanner.line = 1;
}

void Lexer::reset() {
  scanner.current = scanner.start;
  scanner.column = 1;
  scanner.line = 1;
}

char Lexer::peekNext() { return scanner.current[1]; }
char Lexer::advance() {
  scanner.current++;
  scanner.column++;
  return scanner.current[-1];
}
char Lexer::peek() { return *scanner.current; }

bool Lexer::isAtEnd() { return *scanner.current == '\0'; }

const char *Lexer::lineStart(int line) {
  const char *start = scanner.source;
  int cLine = 1;

  while (cLine != line) {
    if (*start == '\n')
      cLine++;
    start++;
  }

  return start;
}

bool Lexer::match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;

  scanner.current++;
  scanner.column++;
  return true;
}

Lexer::Token Lexer::errorToken(std::string message) {
  std::vector<Lexer::Token> tokens = {};
  ErrorClass::error(token.line, token.column, message, "", "Lexer Error",
                    "main.zu", *this, tokens, false, false, true, false, false);
  return makeToken(TokenKind::ERROR_);
}

Lexer::Token Lexer::makeToken(TokenKind kind) {
  token.value = std::string(scanner.start, scanner.current);
  token.column = scanner.column;
  token.start = scanner.start;
  token.line = scanner.line;
  token.kind = kind;
  return token;
}

Lexer::Token Lexer::number() {
  while (isdigit(peek()))
    advance();

  if (peek() == '.' && isdigit(peekNext())) {
    advance();
    while (isdigit(peek()))
      advance();
  }

  return makeToken(TokenKind::NUMBER);
}

Lexer::Token Lexer::String() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      token.line++;
    advance();
  }
  if (isAtEnd())
    return errorToken("Unterminated string.");
  advance(); // The closing ".
  return makeToken(TokenKind::STRING);
}

Lexer::Token Lexer::identifier() {
  while (isalpha(peek()) || isdigit(peek()) || peek() == '_')
    advance();
  return makeToken(identifierType());
}

TokenKind Lexer::identifierType() {
  std::string keyword(scanner.start, scanner.current);
  auto it = keywords.find(keyword);
  if (it != keywords.end())
    return it->second;
  return TokenKind::IDENTIFIER;
}

void Lexer::skipWhitespace() {
  for (;;) {
    char c = peek();

    auto it = whiteSpaceMap.find(c);
    if (it != whiteSpaceMap.end()) {
      it->second(*this);
    } else
      break;
  }
}

const char *Lexer::tokenToString(TokenKind kind) {
  auto it = tokenMap.find(kind);
  if (it != tokenMap.end())
    return it->second;
  return "Unknown";
}

Lexer::Token Lexer::scanToken() {
  skipWhitespace();

  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TokenKind::END_OF_FILE);

  char c = Lexer::advance();

  if (isalpha(c))
    return identifier();
  if (isdigit(c))
    return number();
  if (c == '"')
    return String();

  auto it2 = dcMap.find(std::string(1, c) + std::string(1, peek()));
  if (it2 != dcMap.end()) {
    advance();
    return makeToken(it2->second);
  }

  auto it = scMap.find(c);
  if (it != scMap.end())
    return makeToken(it->second);

  std::string msg = "Unexpected character: " + std::string(1, c);
  return errorToken(msg);
}
