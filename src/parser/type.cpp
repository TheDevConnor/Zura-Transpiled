#include "../helper/error/error.hpp"
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

  // Expect the type of the array
  if (psr->current(psr).kind != TokenKind::IDENTIFIER) {
    std::string msg = "Expected a type for the array!";
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column, msg, "",
                        "Parser Error", node.current_file, lexer, psr->tks, true, false,
                        false, false, false, false);
    return nullptr;
  }
   
  // Else you are good and can continue
  Node::Type *underlying = parseType(psr, defaultValue);
  return new ArrayType(underlying, -1); // -1 is a any-length array
}

Node::Type *Parser::pointer_type(PStruct *psr) {
  psr->advance(psr);
  Node::Type *underlying = parseType(psr, defaultValue);
  return new PointerType(underlying);
}

Node::Type *Parser::type_application(PStruct *psr) {
  psr->advance(psr); // Skip the <
  Node::Type *left = parseType(psr, defaultValue);
  psr->expect(psr, TokenKind::GREATER,
              "Expected a greater than symbol after a type application!");
  Node::Type *right = parseType(psr, defaultValue);
  return new TemplateStructType(right, left);
}

Node::Type *Parser::function_type(PStruct *psr) {
  psr->advance(psr); // Skip the fn
  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a left parenthesis after a function type!");
  std::vector<Node::Type *> args;
  while (psr->peek(psr).kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseType(psr, defaultValue));
    if (psr->peek(psr).kind == TokenKind::COMMA) {
      psr->advance(psr);
    }
  }
  psr->advance(psr); // Skip the right parenthesis
  Node::Type *ret = parseType(psr, defaultValue);
  
  return new FunctionType(args, ret);
}