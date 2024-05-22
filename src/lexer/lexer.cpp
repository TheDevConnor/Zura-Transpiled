#include <unordered_map>

#include "../helper/error/error.hpp"
#include "maps.hpp"
#include "lexer.hpp"

Lexer::Lexer(const char *source) {
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

const char *Lexer::lineStart(int line, const char *source) {
  const char *start = source;
  int cLine = 1;

  while (cLine != line) {
    if (*start == '\0') return "Line not found";
    if (*start == '\n') cLine++;
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
  ErrorClass::error(token, message, *this);
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
  while (isalpha(peek()) || isdigit(peek()))
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
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      scanner.line++;
      scanner.column = 0;
      advance();
      break;
    case '/':
      advance();
      // check for a comment (//) and a block comment (/*)
      if (peek() == '/') {
        while (peek() != '\n' && !isAtEnd())
          advance();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Lexer::Token Lexer::scanToken() {
  skipWhitespace();

  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TokenKind::END_OF_FILE);

  char c = Lexer::advance();

  if (isalpha(c)) return identifier();
  if (isdigit(c)) return number();
  if (c == '"') return String();

  auto it2 = dcMap.find(std::string(1, c) + std::string(1, peek()));
  if (it2 != dcMap.end()) {
    advance();
    return makeToken(it2->second);
  }
  
  auto it = scMap.find(c);
  if (it != scMap.end())
    return makeToken(it->second);

  ErrorClass::error(token, "Unexpected character: " + std::string(1, c), *this);
  return makeToken(TokenKind::ERROR_);
}
