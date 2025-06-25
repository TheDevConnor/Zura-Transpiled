#include "../helper/error/error.hpp"
#include "../ast/types.hpp"
#include "parser.hpp"

Node::Type *Parser::parseType(PStruct *psr) {
  Node::Type *left = type_nud(psr);

  // TODO: Fix the weird bug with the getBP function for types
  // while (type_getBP(psr->current(psr).kind) > bp){
  //     left = type_led(psr, left, type_getBP(psr->current(psr).kind));
  // }

  return left;
}
Node::Type *Parser::symbol_table(PStruct *psr) {
  // check if the next values are a ? or a ! for singed or unsigned
  std::string name = psr->expect(TokenKind::IDENTIFIER, "Expected an identifier for a symbol table!").value;
  switch(psr->peek().kind) {
    case TokenKind::BANG:
      psr->advance();
      return new SymbolType(name, SymbolType::Signedness::UNSIGNED);
    case TokenKind::QUESTION:
      psr->advance();
      return new SymbolType(name, SymbolType::Signedness::SIGNED);
    default:
      return new SymbolType(name);
  }
}

Node::Type *Parser::array_type(PStruct *psr) {
  psr->advance();
  // Check if the next token is an integer (const size)
  size_t size = 0; // 0 is default, uninitialized
  if (psr->peek().kind == TokenKind::INT) {
    // There will be warnings and possible
    // overflow because stoi returns a 32-bit
    // while size is only 16-bit
    size = (size_t)std::stoi(psr->advance().value);
  }
  psr->expect(TokenKind::RIGHT_BRACKET, "Expected a right bracket after an array type!");

  // Expect the type of the array
  if (psr->current().kind != TokenKind::IDENTIFIER) {
    std::string msg = "Expected a type for the array!";
    Error::handle_error("Parser", psr->current_file, msg, psr->tks,
                        psr->current().line, psr->current().column, psr->current().column + psr->current().value.size());
    return nullptr;
  }
   
  // Else you are good and can continue
  Node::Type *underlying = parseType(psr);
  return new ArrayType(underlying, (long long)size);
}

Node::Type *Parser::pointer_type(PStruct *psr) {
  psr->advance();
  Node::Type *underlying = parseType(psr);
  return new PointerType(underlying);
}

Node::Type *Parser::type_application(PStruct *psr) {
  psr->advance(); // Skip the <
  Node::Type *left = parseType(psr);
  psr->expect(TokenKind::GREATER, "Expected a greater than symbol after a type application!");
  Node::Type *right = parseType(psr);
  return new TemplateStructType(right, left);
}

Node::Type *Parser::function_type(PStruct *psr) {
  psr->advance(); // Skip the function keyword
  psr->expect(TokenKind::LEFT_PAREN, "Expected a left parenthesis after a function type!");
  std::vector<Node::Type *> args;
  while (psr->peek().kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseType(psr));
    if (psr->peek().kind == TokenKind::RIGHT_PAREN) break;
    psr->expect(TokenKind::COMMA, "Expected a comma after an argument in a function type!");
  }
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a right parenthesis after a function type!");
  Node::Type *ret = parseType(psr);
  
  return new FunctionType(args, ret);
}