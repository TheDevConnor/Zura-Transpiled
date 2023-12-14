#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include <vector>

class Parser {
public:
  Parser(const char *source);
  ~Parser();

  AstNode *parse();

private:
  const char *source;
  Lexer::Token previousToken;
  Lexer::Token currentToken;

  // Error handling
  bool hadError = false;

  // Expressions
  AstNode *binary(AstNode *left, int precedence);
  AstNode *expression(int precedence = 0);
  AstNode *grouping();
  AstNode *literal();
  AstNode *unary();

  // Main function
  std::vector<AstNode *> lookupMain();

  // Statements
  AstNode *functionDeclaration();
  AstNode *expressionStatement();
  AstNode *varDeclaration();
  AstNode *printStatement();
  AstNode *blockStatement();
  AstNode *exitStatement();
  AstNode *declaration();
  AstNode *statement();

  // Types
  AstNode *findType(AstNode *type);

  // Helper
  bool match(TokenKind kinds);

  void consume(TokenKind kind, std::string message);
  void advance();
  Lexer::Token peek(int offset);
  void synchronize();

  int getPrecedence();
};
