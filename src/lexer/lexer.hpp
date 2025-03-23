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
  CHAR,
  INT,
  FLOAT,

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
  PRINTLN,
  RETURN,
  SUPER,
  TR,
  VAR,
  _CONST,
  PKG,
  TYPE,
  EXIT, // Create an exit syscall from anywhere within the program, whether that be the main function or some other crazy place.
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
  CALL, // Call extern'd functions.
  LINK,
  EXTERN,
  MATCH, // a C-like switch statement
  DEFAULT,
  CASE,
  INPUT,
  READ,
  WRITE,
  ALLOC,
  FREE,
  MEMCPY,
  SIZEOF,
  OPEN,  // open a file and return a fd from its path
  CLOSE, // close a fd

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
  } token;

  struct Scanner {
    const char *current;
    const char *source;
    const char *start;
    std::string file;
    int column;
    int line;
  } scanner;

  void initLexer(const char *source, std::string file);

  Token scanToken(void);
  Token errorToken(std::string message);

  const char *tokenToString(TokenKind kind);

  const char *lineStart(int line);

  void reset(void);
  char advance(void);
  bool isAtEnd(void);
  char peek(void);

  using WhiteSpaceFunction = std::function<void(Lexer &)>;
  std::unordered_map<TokenKind, const char *> tokenToStringMap;
  std::unordered_map<char, WhiteSpaceFunction> whiteSpaceMap;
  std::unordered_map<std::string, TokenKind> at_keywords;
  std::unordered_map<std::string, TokenKind> keywords;
  std::unordered_map<std::string, TokenKind> dcMap;
  std::unordered_map<char, TokenKind> scMap;

  void initMap(void);

private:
  bool match(char expected);

  Token makeToken(TokenKind kind);
  Token identifier(void);
  Token number(void);
  Token String(void);
  Token Char(void);

  TokenKind checkIdentMap(std::string identifier);
  TokenKind sc_dc_lookup(char c);

  void skipWhitespace(void);
};