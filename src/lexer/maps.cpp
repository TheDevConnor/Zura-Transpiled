#include <functional>
#include <string>
#include <unordered_map>

#include "lexer.hpp"

/**
 * Initializes the lexer with the given source code and file path.
 *
 * @param source The source code to be lexed.
 * @param file The file path of the source code.
 */
void Lexer::initLexer(const char *source, std::string file) {
  scanner.current = source;
  scanner.source = source;
  scanner.start = source;
  scanner.column = 1;
  scanner.line = 1;
  scanner.file = file;

  initMap(); // Create the maps when the lexer is initialized
}

/**
 * Initializes the map for the Lexer class.
 * This map contains mappings for whitespace characters, keywords, special
 * characters, and token kinds. The whitespace map is used to handle whitespace
 * characters and advance the lexer accordingly. The keywords map is used to map
 * keywords to their corresponding token kinds. The scMap (special character
 * map) is used to map special characters to their corresponding token kinds.
 * The dcMap (double character map) is used to map double characters (e.g.,
 * "==", "!=") to their corresponding token kinds. The tokenToStringMap is used
 * to map token kinds to their string representations.
 */
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
      {'#',
       [](Lexer &lexer) {
         while (lexer.peek() != '\n' && !lexer.isAtEnd())
           lexer.advance();
       }},
  };

  at_keywords = {
      {"@template", TokenKind::TEMPLATE}, {"@typealias", TokenKind::TYPEALIAS},
      {"@cast", TokenKind::CAST},
  };

  keywords = {
      {"and", TokenKind::AND},           {"else", TokenKind::ELSE},
      {"false", TokenKind::FAL},         {"fn", TokenKind::FUN},
      {"loop", TokenKind::LOOP},         {"if", TokenKind::IF},
      {"nil", TokenKind::NIL},           {"or", TokenKind::OR},
      {"dis", TokenKind::PRINT},         {"return", TokenKind::RETURN},
      {"exit", TokenKind::EXIT},         {"super", TokenKind::SUPER},
      {"true", TokenKind::TR},           {"have", TokenKind::VAR},
      {"pkg", TokenKind::PKG},           {"in", TokenKind::IN},
      {"type", TokenKind::TYPE},         {"struct", TokenKind::STRUCT},
      {"enum", TokenKind::ENUM},         {"union", TokenKind::UNION},
      {"const", TokenKind::_CONST},      {"import", TokenKind::IMPORT},
      {"pub", TokenKind::PUB},           {"priv", TokenKind::PRIV},
      {"break", TokenKind::BREAK},       {"continue", TokenKind::CONTINUE},
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
      {"::", TokenKind::RESOLUTION},
      {"<-", TokenKind::LEFT_ARROW},
      {"->", TokenKind::RIGHT_ARROW},
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
      {TokenKind::INT, "INT"},
      {TokenKind::FLOAT, "FLOAT"},
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
      {TokenKind::CAST, "CAST"},
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