#include <cctype>
#include <string>
#include <unordered_map>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "lexer.hpp"

void Lexer::reset() {
  scanner.current = scanner.start;
  scanner.column = 1;
  scanner.line = 1;
}

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
                    scanner.file, *this, tokens, false, false, true, false,
                    false, false);
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

  if (peek() == '.') {
    advance();
    while (isdigit(peek()))
      advance();
    return makeToken(TokenKind::FLOAT);
  }

  return makeToken(TokenKind::INT);
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

Lexer::Token Lexer::Char() {
  if (isAtEnd())
    return errorToken("Unterminated character literal.");

  char value = advance(); // Get the character value.

  // Check for escaped characters.
  if (value == '\\') {
    if (isAtEnd())
      return errorToken("Unterminated escape sequence in character literal.");
    value = advance(); // Consume the escaped character.
  }

  // Ensure the closing quote is present.
  if (peek() != '\'')
    return errorToken("Unterminated character literal.");
  
  advance(); // Consume the closing quote.
  
  // Validate the length of the character.
  if (scanner.current - scanner.start > 3) // 3: opening `'`, the char, and closing `'`
    return errorToken("Character literal must contain exactly one character.");

  return makeToken(TokenKind::CHAR);
}

Lexer::Token Lexer::identifier() {
  while (isalpha(peek()) || isdigit(peek()) || peek() == '_')
    advance();
  std::string identifier(scanner.start, scanner.current);
  return makeToken(checkIdentMap(identifier));
}

void Lexer::skipWhitespace() {
  for (;;) {
    char c = peek();
    std::unordered_map<char, Lexer::WhiteSpaceFunction>::iterator it = whiteSpaceMap.find(c);
    if (it != whiteSpaceMap.end()) {
      it->second(*this);
    } else
      break;
  }
}

Lexer::Token Lexer::scanToken() {
  skipWhitespace();

  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TokenKind::END_OF_FILE);

  char c = Lexer::advance();

  if (isalpha(c)) return makeToken(identifier().kind);
  if (c == '@')   return makeToken(at_keywords[identifier().value]);
  if (isdigit(c)) return makeToken(number().kind);
  if (c == '"')   return makeToken(String().kind);
  if (c == '\'')  return makeToken(Char().kind);

  auto kind = sc_dc_lookup(c);
  if (kind != TokenKind::UNKNOWN) return makeToken(kind);

  return errorToken("Unexpected character: " + std::string(1, c));
}
