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
  scanner.column = 0;
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
  at_keywords = {
      {"@template", TokenKind::TEMPLATE},
      {"@cast", TokenKind::CAST},
      {"@import", TokenKind::IMPORT},
      {"@link", TokenKind::LINK},
      {"@extern", TokenKind::EXTERN},
      {"@call", TokenKind::CALL},
      {"@output", TokenKind::PRINT},
      {"@read", TokenKind::READ},
      {"@input", TokenKind::INPUT},
      {"@write", TokenKind::WRITE},
      {"@free", TokenKind::FREE},
      {"@alloc", TokenKind::ALLOC},
      {"@memcpy", TokenKind::MEMCPY},
      {"@sizeof", TokenKind::SIZEOF},
      {"@getArgv", TokenKind::GETARGV},
      {"@getArgc", TokenKind::GETARGC},
      {"@streq", TokenKind::STRCMP},
      {"@command", TokenKind::COMMAND},
      // file management
      {"@open", TokenKind::OPEN},
      {"@close", TokenKind::CLOSE},
      {"@outputln", TokenKind::PRINTLN},
      {"@socket", TokenKind::SOCKET},
      {"@bind", TokenKind::BIND},
      {"@listen", TokenKind::LISTEN},
      {"@accept", TokenKind::ACCEPT},
      {"@recv", TokenKind::RECV},
      {"@send", TokenKind::SEND},
  };

  keywords = {
      {"and", TokenKind::AND},
      {"else", TokenKind::ELSE},
      {"false", TokenKind::FAL},
      {"fn", TokenKind::FUN},
      {"loop", TokenKind::LOOP},
      {"if", TokenKind::IF},
      {"nil", TokenKind::NIL},
      {"or", TokenKind::OR},
      {"exit", TokenKind::EXIT},
      {"super", TokenKind::SUPER},
      {"true", TokenKind::TR},
      {"have", TokenKind::VAR},
      {"pkg", TokenKind::PKG},
      {"in", TokenKind::IN},
      {"type", TokenKind::TYPE},
      {"struct", TokenKind::STRUCT},
      {"enum", TokenKind::ENUM},
      {"union", TokenKind::UNION},
      {"const", TokenKind::_CONST},
      {"import", TokenKind::IMPORT},
      {"pub", TokenKind::PUB},
      {"priv", TokenKind::PRIV},
      {"break", TokenKind::BREAK},
      {"continue", TokenKind::CONTINUE},
      {"typename", TokenKind::TYPEALIAS},
      {"match", TokenKind::MATCH},
      {"default", TokenKind::DEFAULT},
      {"case", TokenKind::CASE},
      {"return", TokenKind::RETURN},
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
      {TokenKind::CALL, "CALL"},
      {TokenKind::LINK, "LINK"},
      {TokenKind::EXTERN, "EXTERN"},
      {TokenKind::ERROR_, "ERROR_"},
      {TokenKind::UNKNOWN, "UNKNOWN"},
      {TokenKind::END_OF_FILE, "END_OF_FILE"},
      {TokenKind::TEMPLATE, "TEMPLATE"},
      {TokenKind::TYPEALIAS, "TYPEALIAS"},
      {TokenKind::PUB, "PUB"},
      {TokenKind::PRIV, "PRIV"},
      {TokenKind::BREAK, "BREAK"},
      {TokenKind::CONTINUE, "CONTINUE"},
      {TokenKind::GETARGC, "GETARGC"},
      {TokenKind::GETARGV, "GETARGV"},
  };
}

const char *Lexer::tokenToString(TokenKind kind) {
  if (!tokenToStringMap.contains(kind)) {
    return "UNKNOWN";
  }
  return tokenToStringMap.at(kind);
}

TokenKind Lexer::checkIdentMap(std::string identifier) {
  std::unordered_map<std::string, TokenKind>::iterator it =
      keywords.find(identifier);
  if (it != keywords.end())
    return it->second;
  return TokenKind::IDENTIFIER;
}

TokenKind Lexer::sc_dc_lookup(char c) {
  std::unordered_map<std::string, TokenKind>::iterator dc =
      dcMap.find(std::string(1, c) + std::string(1, peek()));
  if (dc != dcMap.end()) {
    advance();
    return dc->second;
  }

  std::unordered_map<char, TokenKind>::iterator sc = scMap.find(c);
  if (sc != scMap.end())
    return sc->second;

  return TokenKind::UNKNOWN;
}
