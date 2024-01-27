#pragma once

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include <vector>

class Parser {
public:
  Parser(const char *source, Lexer &lexer);

  Lexer &lexer;

  std::unique_ptr<AstNode> parse();

private:
  const char *source;
  Lexer::Token previousToken;
  Lexer::Token currentToken;

  // Error handling
  bool hadError = false;

  // Expressions
  std::unique_ptr<ExprAST> expression(int precedence = 0);
  std::unique_ptr<ExprAST> binary(std::unique_ptr<ExprAST> left,
                                  int precedence);
  std::unique_ptr<ExprAST> unary();
  std::unique_ptr<ExprAST> literal();
  std::unique_ptr<ExprAST> identifier();
  std::unique_ptr<ExprAST> grouping();
  std::unique_ptr<ExprAST> assignment();

  std::vector<std::unique_ptr<StmtAST>> getProgram();

  // Statements
  std::unique_ptr<StmtAST> functionDeclaration();
  std::unique_ptr<StmtAST> expressionStatement();
  std::unique_ptr<StmtAST> returnStatement();
  std::unique_ptr<StmtAST> varDeclaration();
  std::unique_ptr<StmtAST> printStatement();
  std::unique_ptr<StmtAST> blockStatement();
  std::unique_ptr<StmtAST> exitStatement();
  std::unique_ptr<StmtAST> declaration();
  std::unique_ptr<StmtAST> statement();

  // Types
  std::string findType(TokenKind kind);
  TypeAST *buildType(std::string type);

  // Helper
  TokenKind checkType();
  Lexer::Token peek(int offset);

  bool match(TokenKind kinds);

  void consume(TokenKind kind, std::string message);
  void advance();
  void synchronize();

  int getPrecedence();
};
