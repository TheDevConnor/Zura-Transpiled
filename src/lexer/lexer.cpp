#include <unordered_map>

#include "../helper/error/lexerError.hpp"
#include "lexer.hpp"

void Lexer::initToken(const char *source) {
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

Lexer::~Lexer() {
  delete[] scanner.source;
  delete[] scanner.current;
  delete[] scanner.start;
  delete[] token.start;
}

char Lexer::peekNext() { return scanner.current[1]; }
char Lexer::advance() {
  scanner.current++;
  scanner.column++;
  return scanner.current[-1];
}
char Lexer::peek() { return *scanner.current; }

const char *Lexer::lineStart(int line) {
  const char *start = scanner.source;
  int currentLine = 1;

  while (currentLine != line) {
    if (*start == '\n')
      currentLine++;
    start++;
  }

  return start;
}

bool Lexer::match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;

  token.current++;
  token.column++;
  return true;
}

bool Lexer::isAtEnd() { return *scanner.current == '\0'; }

Lexer::Token Lexer::errorToken(std::string message) {
  LexerError::error(message, scanner.line, scanner.column);
  return makeToken(TokenKind::ERROR_);
}

Lexer::Token Lexer::makeToken(TokenKind kind) {
  token.column = scanner.column;
  token.start = scanner.start;
  token.line = scanner.line;
  token.kind = kind;
  return token;
}

Lexer::Token Lexer::identifier() {
  while (isalpha(peek()) || isdigit(peek()))
    advance();
  return makeToken(identifierType());
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

TokenKind Lexer::checkKeyword(std::string identifier) {
  const std::unordered_map<std::string, TokenKind> keywords = {
      {"and", TokenKind::AND},
      {"class", TokenKind::CLASS},
      {"else", TokenKind::ELSE},
      {"false", TokenKind::FAL},
      {"fn", TokenKind::FUN},
      {"loop", TokenKind::LOOP},
      {"if", TokenKind::IF},
      {"nil", TokenKind::NIL},
      {"or", TokenKind::OR},
      {"info", TokenKind::PRINT},
      {"return", TokenKind::RETURN},
      {"exit", TokenKind::EXIT},
      {"super", TokenKind::SUPER},
      {"this", TokenKind::THS},
      {"true", TokenKind::TR},
      {"have", TokenKind::VAR},
      {"pkg", TokenKind::PKG},
      {"type", TokenKind::TYPE},
      {"struct", TokenKind::STRUCT},

      // Types.
      {"i8", TokenKind::I8},
      {"i16", TokenKind::I16},
      {"i32", TokenKind::I32},
      {"i64", TokenKind::I64},
      {"i128", TokenKind::I128},
      {"u8", TokenKind::U8},
      {"u16", TokenKind::U16},
      {"u32", TokenKind::U32},
      {"u64", TokenKind::U64},
      {"u128", TokenKind::U128},
      {"f32", TokenKind::F32},
      {"f64", TokenKind::F64},
      {"f128", TokenKind::F128},
      {"bool", TokenKind::Bool},
      {"char", TokenKind::Char},
      {"str", TokenKind::STRING},
  };

  auto it = keywords.find(identifier);
  if (it != keywords.end())
    return it->second;
  return TokenKind::IDENTIFIER;
}

TokenKind Lexer::identifierType() {
  std::string keyword(scanner.start, scanner.current);
  return checkKeyword(keyword.c_str());
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
      if (peekNext() == '/') {
        // A comment goes until the end of the line.
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

  if (isalpha(c))
    return identifier();
  if (isdigit(c))
    return number();

  switch (c) {
  case '(':
    return makeToken(TokenKind::LEFT_PAREN);
  case ')':
    return makeToken(TokenKind::RIGHT_PAREN);
  case '{':
    return makeToken(TokenKind::LEFT_BRACE);
  case '}':
    return makeToken(TokenKind::RIGHT_BRACE);
  case ';':
    return makeToken(TokenKind::SEMICOLON);
  case ',':
    return makeToken(TokenKind::COMMA);
  case '.':
    return makeToken(TokenKind::DOT);
  case '-':
    return makeToken(TokenKind::MINUS);
  case '+':
    return makeToken(TokenKind::PLUS);
  case '/':
    return makeToken(TokenKind::SLASH);
  case '*':
    return makeToken(TokenKind::STAR);
  case '%':
    return makeToken(TokenKind::MODULO);
  case '^':
    return makeToken(TokenKind::CARET);
  case ':':
    return makeToken(TokenKind::COLON);
  case '=':
    return makeToken(TokenKind::EQUAL);
  case '!':
    return makeToken(match('=') ? TokenKind::BANG_EQUAL : TokenKind::BANG);
  case '<':
    return makeToken(match('=') ? TokenKind::LESS_EQUAL : TokenKind::LESS);
  case '>':
    return makeToken(match('=') ? TokenKind::GREATER_EQUAL
                                : TokenKind::GREATER);
  case '"':
    return String();
  }

  LexerError::error("Unexpected character. " + std::to_string(c), scanner.line,
                    scanner.column);
  return makeToken(TokenKind::ERROR_);
}
