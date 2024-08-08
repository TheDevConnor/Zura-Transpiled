#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

using namespace Parser;

/**
 * @brief Looks up a value in a map based on a key.
 * 
 * This function searches for a key-value pair in the given map and returns the corresponding value.
 * If the key is found, the function returns the associated value.
 * If the key is not found, the function returns nullptr and generates an error message.
 * 
 * @tparam T The type of the value in the map.
 * @tparam U The type of the key in the map.
 * @param psr A pointer to the PStruct object.
 * @param lu The map containing key-value pairs.
 * @param key The key to search for in the map.
 * @return The value associated with the key, or nullptr if the key is not found.
 */
template <typename T, typename U>
T Parser::lookup(PStruct *psr, const std::vector<std::pair<U, T>> &lu, U key) {
  auto it = std::find_if(lu.begin(), lu.end(),
                         [key](auto &p) { return p.first == key; });

  if (isIgnoreToken(key)) {
    return nullptr;
  }

  if (it == lu.end()) {
    ErrorClass::error(0, 0, "No value found for key Expr Maps!", "",
                      "Parser Error", psr->current_file, lexer, psr->tks, true,
                      false, false, false, false);
    return nullptr;
  }

  return it->second;
}

/**
 * @brief Creates the maps used by the Parser class.
 * 
 * This function initializes the lookup tables used by the Parser class for parsing tokens.
 * It sets up the stmt_lu, nud_lu, led_lu, bp_lu, and ignore_tokens maps with their respective values.
 * These maps are used during the parsing process to determine the behavior of different tokens.
 */
void Parser::createMaps() {
  stmt_lu = {
      {TokenKind::_CONST, constStmt},     {TokenKind::VAR, varStmt},
      {TokenKind::LEFT_BRACE, blockStmt}, {TokenKind::FUN, funStmt},
      {TokenKind::RETURN, returnStmt},    {TokenKind::IF, ifStmt},
      {TokenKind::STRUCT, structStmt},    {TokenKind::ENUM, enumStmt},
      {TokenKind::LOOP, loopStmt},        {TokenKind::PRINT, printStmt},
      {TokenKind::IMPORT, importStmt},
  };
  nud_lu = {
      {TokenKind::NUMBER, primary},     {TokenKind::IDENTIFIER, primary},
      {TokenKind::STRING, primary},     {TokenKind::LEFT_PAREN, group},
      {TokenKind::MINUS, unary},        {TokenKind::PLUS_PLUS, _prefix},
      {TokenKind::BANG, unary},         {TokenKind::MINUS_MINUS, _prefix},
      {TokenKind::LEFT_BRACKET, array}, {TokenKind::TR, bool_expr},
      {TokenKind::FAL, bool_expr},
  };
  led_lu = {
      {TokenKind::PLUS, binary},
      {TokenKind::MINUS, binary},

      {TokenKind::STAR, binary},
      {TokenKind::SLASH, binary},
      {TokenKind::CARET, binary},
      {TokenKind::MODULO, binary},

      {TokenKind::EQUAL_EQUAL, binary},
      {TokenKind::BANG_EQUAL, binary},
      {TokenKind::GREATER, binary},
      {TokenKind::GREATER_EQUAL, binary},
      {TokenKind::LESS, binary},
      {TokenKind::LESS_EQUAL, binary},

      {TokenKind::EQUAL, assign},
      {TokenKind::PLUS_EQUAL, assign},
      {TokenKind::MINUS_EQUAL, assign},
      {TokenKind::STAR_EQUAL, assign},
      {TokenKind::SLASH_EQUAL, assign},

      {TokenKind::QUESTION, _ternary},
      {TokenKind::COLON, _ternary},

      {TokenKind::LEFT_PAREN, parse_call},

      {TokenKind::DOT, _member},
      {TokenKind::RESOLUTION, resolution},

      {TokenKind::RANGE, binary},

      {TokenKind::AND, binary},
      {TokenKind::OR, binary},
  };
  bp_lu = {
      {TokenKind::COMMA, BindingPower::comma},
      {TokenKind::EQUAL, BindingPower::assignment},

      {TokenKind::OR, BindingPower::logicalOr},
      {TokenKind::AND, BindingPower::logicalAnd},

      {TokenKind::BANG, BindingPower::prefix},
      {TokenKind::EQUAL_EQUAL, BindingPower::comparison},
      {TokenKind::BANG_EQUAL, BindingPower::comparison},
      {TokenKind::GREATER, BindingPower::comparison},
      {TokenKind::GREATER_EQUAL, BindingPower::comparison},
      {TokenKind::LESS, BindingPower::comparison},
      {TokenKind::LESS_EQUAL, BindingPower::comparison},

      {TokenKind::PLUS, BindingPower::additive},
      {TokenKind::MINUS, BindingPower::additive},

      {TokenKind::STAR, BindingPower::multiplicative},
      {TokenKind::SLASH, BindingPower::multiplicative},
      {TokenKind::MODULO, BindingPower::multiplicative},

      {TokenKind::CARET, BindingPower::power},

      {TokenKind::LEFT_PAREN, BindingPower::call},

      {TokenKind::EQUAL, BindingPower::assignment},
      {TokenKind::PLUS_EQUAL, BindingPower::assignment},
      {TokenKind::MINUS_EQUAL, BindingPower::assignment},
      {TokenKind::STAR_EQUAL, BindingPower::assignment},
      {TokenKind::SLASH_EQUAL, BindingPower::assignment},

      {TokenKind::QUESTION, BindingPower::ternary},

      {TokenKind::PLUS_PLUS, BindingPower::prefix},
      {TokenKind::MINUS_MINUS, BindingPower::prefix},

      {TokenKind::IDENTIFIER, BindingPower::defaultValue},
      {TokenKind::NUMBER, BindingPower::defaultValue},
      {TokenKind::STRING, BindingPower::defaultValue},

      {TokenKind::RANGE, BindingPower::range},

      {TokenKind::DOT, BindingPower::member},
      {TokenKind::RESOLUTION, BindingPower::member},
  };
  ignore_tokens = {
      TokenKind::SEMICOLON,
      TokenKind::COMMA,
      TokenKind::COLON,
      TokenKind::WALRUS,
  };
}

/**
 * Checks if a given token is an ignore token.
 *
 * @param tk The token kind to check.
 * @return True if the token is an ignore token, false otherwise.
 */
bool Parser::isIgnoreToken(TokenKind tk) {
  return std::find(ignore_tokens.begin(), ignore_tokens.end(), tk) !=
         ignore_tokens.end();
}

/**
 * @brief Parses a null denotation expression.
 * 
 * This function is responsible for parsing a null denotation expression in the parser.
 * It retrieves the current operator from the parser's PStruct and looks up the appropriate
 * parsing function based on the operator's kind. If a parsing function is found, it is called
 * with the parser as an argument and the result is returned. If no parsing function is found,
 * the parser advances to the next token and returns nullptr.
 * 
 * If an exception of type std::runtime_error is caught during the parsing process, an error
 * message is generated using the ErrorClass::error() function and nullptr is returned.
 * 
 * @param psr A pointer to the parser's PStruct.
 * @return A pointer to the parsed expression, or nullptr if parsing fails.
 */
Node::Expr *Parser::nud(PStruct *psr) {
  auto op = psr->current(psr);
  try {
    auto result = lookup(psr, nud_lu, op.kind);
    if (result == nullptr) {
      psr->advance(psr);
      return nullptr;
    }
    return result(psr);
  } catch (std::runtime_error &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Could not parse expression in NUD!", "", "Parser Error",
                      "main.zu", lexer, psr->tks, true, false, false, false,
                      false);
    return nullptr;
  }
}

/**
 * Parses the left expression with the given binding power using the led (left denotation) rule.
 *
 * @param psr The parser object.
 * @param left The left expression to be parsed.
 * @param bp The binding power of the operator.
 * @return The parsed expression.
 */
Node::Expr *Parser::led(PStruct *psr, Node::Expr *left, BindingPower bp) {
  auto op = psr->current(psr);
  try {
    auto result = lookup(psr, led_lu, op.kind);
    if (result == nullptr) {
      psr->advance(psr);
      return left;
    }
    return result(psr, left, bp);
  } catch (std::runtime_error &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Could not parse expression in LED!", "", "Parser Error",
                      "main.zu", lexer, psr->tks, true, false, false, false,
                      false);
    return nullptr;
  }
}

/**
 * @brief Parses a statement based on the provided PStruct and name.
 * 
 * This function searches for a statement in the stmt_lu map that matches the kind of the current PStruct.
 * If a matching statement is found, it calls the corresponding function and passes the PStruct and name as arguments.
 * If no matching statement is found, it returns nullptr.
 * 
 * @param psr The PStruct object used for parsing.
 * @param name The name of the statement.
 * @return A pointer to the parsed statement, or nullptr if no matching statement is found.
 */
Node::Stmt *Parser::stmt(PStruct *psr, std::string name) {
  auto stmt_it = std::find_if(stmt_lu.begin(), stmt_lu.end(), [psr](auto &p) {
    return p.first == psr->current(psr).kind;
  });
  return stmt_it != stmt_lu.end() ? stmt_it->second(psr, name) : nullptr;
}

BindingPower Parser::getBP(TokenKind tk) {
  auto bp_it = std::find_if(bp_lu.begin(), bp_lu.end(),
                            [tk](auto &p) { return p.first == tk; });

  return bp_it != bp_lu.end() ? bp_it->second : BindingPower::defaultValue;
}