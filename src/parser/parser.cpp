#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

Parser::Parser(const char *source) : source(source) {}

AstNode *Parser::parse() {
  lexer.initToken(source);

  advance();

  std::vector<AstNode *> statements = lookupMain();
  AstNode *main =
      new AstNode(AstNodeType::PROGRAM, new AstNode::Program(statements));

  return main;
}
