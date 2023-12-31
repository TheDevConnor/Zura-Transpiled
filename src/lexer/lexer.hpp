#pragma once

#include <string>

enum TokenKind {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  MODULO,
  CARET,
  COLON,
  STRUCT,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  WALRUS,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FAL,
  FUN,
  LOOP,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THS,
  TR,
  VAR,
  PKG,
  TYPE,
  EXIT,

  // Types.
  I8,
  I16,
  I32,
  I64,
  I128,
  U8,
  U16,
  U32,
  U64,
  U128,
  F32,
  F64,
  F128,
  Bool,
  Char,

  // Error
  ERROR_,
  UNKNOWN,

  // End of file.
  END_OF_FILE
};

class Lexer {
public:

  Lexer(const char *source);

  struct Token {
    const char *start;
    TokenKind kind;
    int current;
    int column;
    int line;
  };

  struct Keyword {
    std::string name;
    TokenKind kind;
  };

  Token scanToken();
  Token token;
  Token errorToken(std::string message);

  const char *lineStart(int line);

  void reset();

private:
  struct Scanner {
    const char *current;
    const char *source;
    const char *start;
    int column;
    int line;
  };

  Scanner scanner;

  char peekNext();
  char advance();
  char peek();

  bool match(char expected);
  bool isAtEnd();

  Token makeToken(TokenKind kind);
  Token identifier();
  Token number();
  Token String();

  TokenKind checkKeyword(std::string identifier);
  TokenKind identifierType();

  void skipWhitespace();
};