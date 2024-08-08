#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

using namespace Parser;

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

bool Parser::isIgnoreToken(TokenKind tk) {
  return std::find(ignore_tokens.begin(), ignore_tokens.end(), tk) !=
         ignore_tokens.end();
}

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