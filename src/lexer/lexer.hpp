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
  QUESTION,
  LAND, 
  LOR, 

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
  PLUS_PLUS,
  MINUS_MINUS,
  PLUS_EQUAL,
  MINUS_EQUAL,
  STAR_EQUAL,
  SLASH_EQUAL,
  RANGE,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
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
  TR,
  VAR,
  _CONST,
  PKG,
  TYPE,
  EXIT,
  IN,
  STRUCT,
  ENUM,
  UNION,

  // Error
  ERROR_,
  UNKNOWN,

  // End of file.
  END_OF_FILE
};

class Lexer {
public:
  struct Token {
    const char *start;
    TokenKind kind;
    std::string value; // This is also know as the lexeme
    int current;
    int column;
    int line;
  };
  Token token;

  struct Scanner {
    const char *current;
    const char *source;
    const char *start;
    int column;
    int line;
  };
  Scanner scanner;

  struct Keyword {
    std::string name;
    TokenKind kind;
  };

  void initLexer(const char *source);

  Token scanToken();
  Token errorToken(std::string message);

  const char *tokenToString(TokenKind kind);

  const char *lineStart(int line);

  void reset();
  char advance();
  bool isAtEnd();
  char peek();

private:
  char peekNext();

  bool match(char expected);

  Token makeToken(TokenKind kind);
  Token identifier();
  Token number();
  Token String();

  TokenKind identifierType();

  void skipWhitespace();
};