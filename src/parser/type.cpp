#include "../ast/types.hpp"
#include "parser.hpp"

#include <iostream>

Node::Type *Parser::parseType(PStruct *psr, BindingPower bp) {
  Node::Type *left = type_nud(psr);

  // TODO: Fix the weird bug with the getBP function for types
  // while (type_getBP(psr->current(psr).kind) > bp){
  //     left = type_led(psr, left, type_getBP(psr->current(psr).kind));
  // }

  return left;
}
Node::Type *Parser::symbol_table(PStruct *psr) {
  return new SymbolType(
      psr->expect(psr, TokenKind::IDENTIFIER,
                  "Expected an identifier for a symbol table!")
          .value);
}

Node::Type *Parser::array_type(PStruct *psr) {
  psr->advance(psr);
  // Check if the next token is an integer (const size)
  signed short int size = -1;
  if (psr->peek(psr).kind == TokenKind::INT) {
    // There will be warnings and possible
    // overflow because stoi returns a 32-bit
    // while size is only 16-bit
    size = std::stoi(psr->advance(psr).value);
  }
  psr->expect(psr, TokenKind::RIGHT_BRACKET,
              "Expected a right bracket after an array type!");
  Node::Type *underlying = parseType(psr, defaultValue);
  return new ArrayType(underlying, -1); // -1 is a variable-length array
}

Node::Type *Parser::pointer_type(PStruct *psr) {
  psr->advance(psr);
  Node::Type *underlying = parseType(psr, defaultValue);
  return new PointerType(underlying);
}