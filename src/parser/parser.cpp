#include <vector>
#include <memory>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

Parser::Parser(const char *source) : source(source) {}

std::unique_ptr<AstNode> Parser::parse() {
  lexer.initToken(source);

  advance();

  std::vector<AstNode *> statements = lookupMain();
  std::unique_ptr<AstNode> main = std::make_unique<AstNode>(AstNodeType::PROGRAM, new AstNode::Program(statements));

  return main;
}
