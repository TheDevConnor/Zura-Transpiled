#include <memory>
#include <vector>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"
#include "../visitor/visit.hpp"
#include "parser.hpp"

Parser::Parser(const char *source, Lexer &lexer)
    : source(source), lexer(lexer) {}

std::unique_ptr<AstNode> Parser::parse() {
  advance();
  std::vector<std::unique_ptr<StmtAST>> statements = getProgram();
  auto main =
      std::make_unique<AstNode>(AstNodeType::PROGRAM, std::move(statements));

  AstVisitor visitor;
  visitor.visit(*main);

  if (!foundMain)
    ErrorClass::error(currentToken,
                      "No 'main' function found! Please add in a main function "
                      "Example: 'fn main() { dis \"hello Wold\"; exit 0;}",
                      lexer);

  return main;
}
