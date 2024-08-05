#include <functional>
#include <string>
#include <unordered_map>

#include "lexer.hpp"

void Lexer::initLexer(const char *source, std::string file) {
  scanner.current = source;
  scanner.source = source;
  scanner.start = source;
  scanner.column = 1;
  scanner.line = 1;
  scanner.file = file;

  initMap(); // Create the maps when the lexer is initialized
}

void Lexer::initMap() {
  whiteSpaceMap = {
      {' ', [](Lexer &lexer) { lexer.advance(); }},
      {'\r', [](Lexer &lexer) { lexer.advance(); }},
      {'\t', [](Lexer &lexer) { lexer.advance(); }},
      {'\n',
       [](Lexer &lexer) {
         lexer.scanner.line++;
         lexer.scanner.column = 0;
         lexer.advance();
       }},
      {'/',
       [](Lexer &lexer) {
         if (lexer.peek() == '/') {
           while (lexer.peek() != '\n' && !lexer.isAtEnd())
             lexer.advance();
         }
       }},
  };

  keywords = {
      {"and", TokenKind::AND},      {"else", TokenKind::ELSE},
      {"false", TokenKind::FAL},    {"fn", TokenKind::FUN},
      {"loop", TokenKind::LOOP},    {"if", TokenKind::IF},
      {"nil", TokenKind::NIL},      {"or", TokenKind::OR},
      {"dis", TokenKind::PRINT},    {"return", TokenKind::RETURN},
      {"exit", TokenKind::EXIT},    {"super", TokenKind::SUPER},
      {"true", TokenKind::TR},      {"have", TokenKind::VAR},
      {"pkg", TokenKind::PKG},      {"in", TokenKind::IN},
      {"type", TokenKind::TYPE},    {"struct", TokenKind::STRUCT},
      {"enum", TokenKind::ENUM},    {"union", TokenKind::UNION},
      {"const", TokenKind::_CONST}, {"import", TokenKind::IMPORT},
      {"pub", TokenKind::PUB},      {"priv", TokenKind::PRIV},
  };

  scMap = {
      {'(', TokenKind::LEFT_PAREN},    {')', TokenKind::RIGHT_PAREN},
      {'{', TokenKind::LEFT_BRACE},    {'}', TokenKind::RIGHT_BRACE},
      {';', TokenKind::SEMICOLON},     {',', TokenKind::COMMA},
      {'.', TokenKind::DOT},           {'-', TokenKind::MINUS},
      {'+', TokenKind::PLUS},          {'/', TokenKind::SLASH},
      {'*', TokenKind::STAR},          {'%', TokenKind::MODULO},
      {'^', TokenKind::CARET},         {'[', TokenKind::LEFT_BRACKET},
      {']', TokenKind::RIGHT_BRACKET}, {'?', TokenKind::QUESTION},
      {':', TokenKind::COLON},         {'=', TokenKind::EQUAL},
      {'!', TokenKind::BANG},          {'<', TokenKind::LESS},
      {'>', TokenKind::GREATER},       {'&', TokenKind::LAND},
      {'|', TokenKind::LOR},
  };

  dcMap = {
      {"!=", TokenKind::BANG_EQUAL},
      {"==", TokenKind::EQUAL_EQUAL},
      {">=", TokenKind::GREATER_EQUAL},
      {"<=", TokenKind::LESS_EQUAL},
      {":=", TokenKind::WALRUS},
      {"++", TokenKind::PLUS_PLUS},
      {"--", TokenKind::MINUS_MINUS},
      {"+=", TokenKind::PLUS_EQUAL},
      {"-=", TokenKind::MINUS_EQUAL},
      {"*=", TokenKind::STAR_EQUAL},
      {"/=", TokenKind::SLASH_EQUAL},
      {"&&", TokenKind::AND},
      {"||", TokenKind::OR},
      {"..", TokenKind::RANGE},
  };

  tokenToStringMap = {
      {TokenKind::LEFT_PAREN, "LEFT_PAREN"},
      {TokenKind::RIGHT_PAREN, "RIGHT_PAREN"},
      {TokenKind::LEFT_BRACE, "LEFT_BRACE"},
      {TokenKind::RIGHT_BRACE, "RIGHT_BRACE"},
      {TokenKind::LEFT_BRACKET, "LEFT_BRACKET"},
      {TokenKind::RIGHT_BRACKET, "RIGHT_BRACKET"},
      {TokenKind::COMMA, "COMMA"},
      {TokenKind::DOT, "DOT"},
      {TokenKind::MINUS, "MINUS"},
      {TokenKind::PLUS, "PLUS"},
      {TokenKind::SEMICOLON, "SEMICOLON"},
      {TokenKind::SLASH, "SLASH"},
      {TokenKind::STAR, "STAR"},
      {TokenKind::MODULO, "MODULO"},
      {TokenKind::CARET, "CARET"},
      {TokenKind::COLON, "COLON"},
      {TokenKind::STRUCT, "STRUCT"},
      {TokenKind::BANG, "BANG"},
      {TokenKind::BANG_EQUAL, "BANG_EQUAL"},
      {TokenKind::EQUAL, "EQUAL"},
      {TokenKind::EQUAL_EQUAL, "EQUAL_EQUAL"},
      {TokenKind::GREATER, "GREATER"},
      {TokenKind::GREATER_EQUAL, "GREATER_EQUAL"},
      {TokenKind::LESS, "LESS"},
      {TokenKind::LESS_EQUAL, "LESS_EQUAL"},
      {TokenKind::WALRUS, "WALRUS"},
      {TokenKind::PLUS_PLUS, "PLUS_PLUS"},
      {TokenKind::MINUS_MINUS, "MINUS_MINUS"},
      {TokenKind::RANGE, "RANGE"},
      {TokenKind::IDENTIFIER, "IDENTIFIER"},
      {TokenKind::STRING, "STRING"},
      {TokenKind::NUMBER, "NUMBER"},
      {TokenKind::AND, "AND"},
      {TokenKind::ELSE, "ELSE"},
      {TokenKind::FAL, "FAL"},
      {TokenKind::FUN, "FUN"},
      {TokenKind::LOOP, "LOOP"},
      {TokenKind::IF, "IF"},
      {TokenKind::NIL, "NIL"},
      {TokenKind::OR, "OR"},
      {TokenKind::PRINT, "PRINT"},
      {TokenKind::RETURN, "RETURN"},
      {TokenKind::SUPER, "SUPER"},
      {TokenKind::TR, "TRUE"},
      {TokenKind::VAR, "VAR"},
      {TokenKind::IN, "IN"},
      {TokenKind::ENUM, "ENUM"},
      {TokenKind::UNION, "UNION"},
      {TokenKind::_CONST, "CONST"},
      {TokenKind::STRUCT, "STRUCT"},
      {TokenKind::PKG, "PKG"},
      {TokenKind::TYPE, "TYPE"},
      {TokenKind::EXIT, "EXIT"},
  };
}

const char *Lexer::tokenToString(TokenKind kind) {
  auto it = tokenToStringMap.find(kind);
  if (it != tokenToStringMap.end())
    return it->second;
  return "Unknown";
}

TokenKind Lexer::checkIdentMap(std::string identifier) {
  auto it = keywords.find(identifier);
  if (it != keywords.end())
    return it->second;
  return TokenKind::IDENTIFIER;
}

TokenKind Lexer::sc_dc_lookup(char c) {
  auto dc = dcMap.find(std::string(1, c) + std::string(1, peek()));
  if (dc != dcMap.end()) {
    advance();
    return dc->second;
  }

  auto sc = scMap.find(c);
  if (sc != scMap.end())
    return sc->second;

  return TokenKind::UNKNOWN;
}