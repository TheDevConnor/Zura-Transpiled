#include "../ast/types.hpp"
#include "parser.hpp"

template <typename T, typename U>
T Parser::lookup(PStruct *psr, const std::vector<std::pair<U, T>> &lu, U key) {
  auto it = std::find_if(lu.begin(), lu.end(),
                         [key](auto &p) { return p.first == key; });

  if (it == lu.end()) {
    ErrorClass::error(0, 0, "No value found for key in Type maps", "",
                      "Parser Error", "main.zu", lexer, psr->tks, true, false,
                      false, false);
  }

  return it->second;
}

void Parser::createTypeMaps() {
  type_nud_lu = {
      {TokenKind::IDENTIFIER, symbol_table},
      {TokenKind::LEFT_BRACKET, array_type},
      {TokenKind::STAR, pointer_type}, // *
      {TokenKind::LAND, pointer_type}, // &
  };
  type_led_lu = {};
  type_bp_lu = bp_lu;
}

Node::Type *Parser::type_led(PStruct *psr, Node::Type *left, BindingPower bp) {
  auto op = psr->current(psr);
  try {
    return lookup(psr, type_led_lu, op.kind)(psr, left, bp);
  } catch (std::exception &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Error in type_led: " + std::string(e.what()), "",
                      "Parser Error", "main.zu", lexer, psr->tks, true, false,
                      false, false);
    return nullptr;
  }
  return nullptr;
}

Node::Type *Parser::type_nud(PStruct *psr) {
  auto op = psr->current(psr);
  try {
    return Parser::lookup(psr, type_nud_lu, op.kind)(psr);
  } catch (std::exception &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Error in type_nud: " + std::string(e.what()), "",
                      "Parser Error", "main.zu", lexer, psr->tks, true, false,
                      false, false);
    return nullptr;
  }
  return nullptr;
}

Parser::BindingPower Parser::type_getBP(PStruct *psr, TokenKind tk) {
  return lookup(psr, type_bp_lu, tk);
}
