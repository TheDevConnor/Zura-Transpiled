#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include <memory>
#include <vector>

class Parser {
public:
  Parser(const char *source);

  std::unique_ptr<AstNode> parse();

private:
  const char *source;
  Lexer::Token previousToken;
  Lexer::Token currentToken;

  // Error handling
  bool hadError = false;

  // Expressions
  std::unique_ptr<AstNode> binary(std::unique_ptr<AstNode> left,
                                  int precedence);
  std::unique_ptr<AstNode> expression(int precedence = 0);
  std::unique_ptr<AstNode> grouping();
  std::unique_ptr<AstNode> literal();
  std::unique_ptr<AstNode> unary();

  // Main function
  std::vector<AstNode *> lookupMain();

  // Statements
  std::unique_ptr<AstNode> functionDeclaration();
  std::unique_ptr<AstNode> expressionStatement();
  std::unique_ptr<AstNode> varDeclaration();
  std::unique_ptr<AstNode> printStatement();
  std::unique_ptr<AstNode> blockStatement();
  std::unique_ptr<AstNode> exitStatement();
  std::unique_ptr<AstNode> declaration();
  std::unique_ptr<AstNode> statement();

  // Types
  std::unique_ptr<AstNode> findType(std::unique_ptr<AstNode> type);

  // Helper
  void consume(TokenKind kind, std::string message);
  Lexer::Token peek(int offset);
  bool match(TokenKind kinds);
  void synchronize();
  void advance();

  int getPrecedence();
};
