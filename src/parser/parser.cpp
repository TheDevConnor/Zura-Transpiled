#include <vector>
#include <memory>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

Parser::Parser(const char *source, Lexer &lexer) : source(source), lexer(lexer) {}

std::unique_ptr<AstNode> Parser::parse() {
  advance();
  std::vector<std::unique_ptr<StmtAST>> statements;

  while (!match(TokenKind::END_OF_FILE)) {
    std::unique_ptr<StmtAST> stmt = declaration();
    statements.push_back(std::move(stmt));
  }

  return std::make_unique<AstNode>(AstNodeType::PROGRAM, std::move(statements));
}
