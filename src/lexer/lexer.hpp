#pragma once

#include <functional>
#include <string>
#include <unordered_map>

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
  RESOLUTION,
  RIGHT_ARROW,
  LEFT_ARROW,
  AT,

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
  IMPORT,
  PUB,
  PRIV,
  TEMPLATE,
  TYPEALIAS,
  BREAK, 
  CONTINUE,
  CAST,

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
    std::string file;
    int column;
    int line;
  };
  Scanner scanner;

  void initLexer(const char *source, std::string file);

  Token scanToken();
  Token errorToken(std::string message);

  const char *tokenToString(TokenKind kind);

  const char *lineStart(int line);

  void reset();
  char advance();
  bool isAtEnd();
  char peek();

  using WhiteSpaceFunction = std::function<void(Lexer &)>;
  std::unordered_map<TokenKind, const char *> tokenToStringMap;
  std::unordered_map<char, WhiteSpaceFunction> whiteSpaceMap;
  std::unordered_map<std::string, TokenKind> keywords;
  std::unordered_map<std::string, TokenKind> dcMap;
  std::unordered_map<char, TokenKind> scMap;

  void initMap();

private:
  bool match(char expected);

  Token makeToken(TokenKind kind);
  Token identifier();
  Token number();
  Token String();

  TokenKind checkIdentMap(std::string identifier);
  TokenKind sc_dc_lookup(char c);

  void skipWhitespace();
};