#include <cctype>
#include <string>
#include <unordered_map>

#include "../helper/error/error.hpp"
#include "lexer.hpp"

void Lexer::reset() {
  scanner.current = scanner.start;
  scanner.column = 1;
  scanner.line = 1;
}

char Lexer::advance() {
  char c = *scanner.current++;
  if (c == '\n') {
    scanner.line++;
    scanner.column = 0;
  } else {
    scanner.column++;
  }

  return c;
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
  if ((*scanner.current != expected) || isAtEnd())
    return false;

  scanner.current++;
  scanner.column++;
  return true;
}

Lexer::Token Lexer::errorToken(std::string message, int whitespace) {
  Error::handle_lexer_error(*this, "Lexer", scanner.file, message);
  return makeToken(TokenKind::ERROR_, whitespace);
}

Lexer::Token Lexer::makeToken(TokenKind kind, int whitespace) {
  token.value = std::string(scanner.start, scanner.current);
  token.column = scanner.column;
  token.start = scanner.start;
  token.line = scanner.line;
  token.kind = kind;
  token.whitespace = whitespace;
  return token;
}

Lexer::Token Lexer::number(int whitespace) {
  while (isdigit(peek()))
    advance();

  if (peek() == '.') {
    advance();
    while (isdigit(peek()))
      advance();
    return makeToken(TokenKind::FLOAT, whitespace);
  }

  return makeToken(TokenKind::INT, whitespace);
}

Lexer::Token Lexer::String(int whitespace) {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      token.line++;
    if (peek() == '\\')
      if (*(scanner.current+1) == '"') {
        advance();
      }
    advance();
  }
  if (isAtEnd())
    return errorToken("Unterminated string.", whitespace);
  advance(); // The closing ".
  return makeToken(TokenKind::STRING, whitespace);
}

Lexer::Token Lexer::Char(int whitespace) {
  if (isAtEnd())
    return errorToken("Unterminated character literal.", whitespace);

  char value = advance(); // Get the character value.

  // Check for escaped characters.
  if (value == '\\') {
    if (isAtEnd())
      return errorToken("Unterminated escape sequence in character literal.", whitespace);
    value = advance(); // Consume the escaped character.
  }

  // Ensure the closing quote is present.
  if (peek() != '\'')
    return errorToken("Unterminated character literal.", whitespace);
  
  advance(); // Consume the closing quote.
  
  // Validate the length of the character.
  if (scanner.current - scanner.start > 3) // 3: opening `'`, the char, and closing `'`
    return errorToken("Character literal must contain exactly one character.", whitespace);

  return makeToken(TokenKind::CHAR, whitespace);
}

Lexer::Token Lexer::identifier(int whitespace) {
  while (isalpha(peek()) || isdigit(peek()) || peek() == '_')
    advance();
  std::string identifier(scanner.start, scanner.current);
  return makeToken(checkIdentMap(identifier), whitespace);
}

int Lexer::skipWhitespace() {
  int count = 0;
  for (;;) {
    char c = peek();
    switch (c) {
      case '#':
        while (peek() != '\n' && !isAtEnd())
          advance();
        break;
      case ' ':
      case '\r':
      case '\t':
        count++;
        advance();
        break;
      case '\n':
        advance();
        break;
      default:
        return count;
    }
  }
}

Lexer::Token Lexer::scanToken() {
  int whitespace_count = skipWhitespace();

  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TokenKind::END_OF_FILE, whitespace_count);

  char c = Lexer::advance();

  if (isalpha(c)) return makeToken(identifier(whitespace_count).kind, whitespace_count);
  if (c == '@') {
    auto ident_token = identifier(whitespace_count);
    auto it = at_keywords.find(ident_token.value);
    if (ident_token.value.empty() || it == at_keywords.end()) {
      return errorToken("Unknown or missing @-keyword after '@'", whitespace_count);
    }
    return makeToken(it->second, whitespace_count);
  }
  if (isdigit(c)) return makeToken(number(whitespace_count).kind, whitespace_count);
  if (c == '"')   return makeToken(String(whitespace_count).kind, whitespace_count);
  if (c == '\'')  return makeToken(Char(whitespace_count).kind, whitespace_count);

  auto kind = sc_dc_lookup(c);
  if (kind != TokenKind::UNKNOWN) return makeToken(kind, whitespace_count);

  return errorToken("Unexpected character: " + std::string(1, c), whitespace_count);
}
