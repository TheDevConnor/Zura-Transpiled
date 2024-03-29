#include <memory>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "parser.hpp"

TokenKind Parser::checkType() {
  switch (currentToken.kind) {
  case TokenKind::I8:
    return TokenKind::I8;
  case TokenKind::I16:
    return TokenKind::I16;
  case TokenKind::I32:
    return TokenKind::I32;
  case TokenKind::I64:
    return TokenKind::I64;
  case TokenKind::U8:
    return TokenKind::U8;
  case TokenKind::U16:
    return TokenKind::U16;
  case TokenKind::U32:
    return TokenKind::U32;
  case TokenKind::U64:
    return TokenKind::U64;
  case TokenKind::F32:
    return TokenKind::F32;
  case TokenKind::F64:
    return TokenKind::F64;
  case TokenKind::Bool:
    return TokenKind::Bool;
  case TokenKind::Char:
    return TokenKind::Char;
  case TokenKind::STRING:
    return TokenKind::STRING;
  default:
    return UNKNOWN;
  }
}

std::unique_ptr<ExprAST> Parser::expression(int precedence) {
  std::unique_ptr<ExprAST> left = unary();

  return binary(std::move(left), precedence);
}

std::unique_ptr<ExprAST> Parser::binary(std::unique_ptr<ExprAST> left,
                                        int precedence) {
  while (true) {
    int currentPrec = getPrecedence();

    if (currentPrec < precedence)
      return left;

    TokenKind op = currentToken.kind;
    advance();

    std::unique_ptr<ExprAST> right = unary();

    left =
        std::make_unique<BinaryExprAST>(std::move(left), std::move(right), op);
  }
}

std::unique_ptr<ExprAST> Parser::unary() {
  if (match(TokenKind::MINUS)) {
    std::unique_ptr<ExprAST> right = unary();
    return std::make_unique<UnaryExprAST>(std::move(right), TokenKind::MINUS);
  }

  if (match(TokenKind::BANG)) {
    std::unique_ptr<ExprAST> right = unary();
    return std::make_unique<UnaryExprAST>(std::move(right), TokenKind::BANG);
  }

  return grouping();
}

std::unique_ptr<ExprAST> Parser::grouping() {
  if (match(TokenKind::LEFT_PAREN)) {
    std::unique_ptr<ExprAST> expr = expression();
    consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
    return std::make_unique<GroupingExprAST>(std::move(expr));
  }

  return literal();
}

std::unique_ptr<ExprAST> Parser::literal() {
  switch (currentToken.kind) {
  case TokenKind::NUMBER: {
    double number = std::stod(currentToken.start);
    advance();
    return std::make_unique<NumberExprAST>(number);
  }
  case TokenKind::STRING: {
    Lexer::Token value = currentToken;
    advance();
    return std::make_unique<LiteralExprAST>(value);
  }
  case TokenKind::TR: {
    Lexer::Token value = currentToken;
    advance();
    return std::make_unique<LiteralExprAST>(value);
  }
  case TokenKind::FAL: {
    Lexer::Token value = currentToken;
    advance();
    return std::make_unique<LiteralExprAST>(value);
  }
  case TokenKind::NIL: {
    Lexer::Token value = currentToken;
    advance();
    return std::make_unique<LiteralExprAST>(value);
  }
  case TokenKind::IDENTIFIER: {
    Lexer::Token value = currentToken;
    advance();
    return std::make_unique<IdentifierExprAST>(value.start);
  }
  default:
    ErrorClass::error(currentToken, "Expected literal", lexer);
  }

  synchronize();
  return nullptr;
}

void Parser::synchronize() {
  advance();

  while (currentToken.kind != TokenKind::END_OF_FILE) {
    if (previousToken.kind == TokenKind::SEMICOLON)
      return;

    switch (currentToken.kind) {
    case TokenKind::CLASS:
    case TokenKind::FUN:
    case TokenKind::VAR:
    case TokenKind::IF:
    case TokenKind::LOOP:
    case TokenKind::PRINT:
    case TokenKind::RETURN:
      return;
    default:
      break;
    }

    advance();
  }
}
