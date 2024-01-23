#include <memory>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "parser.hpp"

TokenKind Parser::checkType() {
  switch (currentToken.kind) {
    case TokenKind::I8: return TokenKind::I8;
    case TokenKind::I16: return TokenKind::I16;
    case TokenKind::I32: return TokenKind::I32;
    case TokenKind::I64: return TokenKind::I64;
    case TokenKind::U8: return TokenKind::U8;
    case TokenKind::U16: return TokenKind::U16;
    case TokenKind::U32: return TokenKind::U32;
    case TokenKind::U64: return TokenKind::U64;
    case TokenKind::F32: return TokenKind::F32;
    case TokenKind::F64: return TokenKind::F64;
    case TokenKind::Bool: return TokenKind::Bool;
    case TokenKind::Char: return TokenKind::Char;
    case TokenKind::STRING: return TokenKind::STRING;
    default: return UNKNOWN;
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

  return literal();
}

std::unique_ptr<ExprAST> Parser::expression(int precedence) {
  std::unique_ptr<ExprAST> left = unary();

  return binary(std::move(left), precedence);
}

std::unique_ptr<ExprAST> Parser::binary(std::unique_ptr<ExprAST> left,
                                        int precedence) {
  TokenKind operatorKind = currentToken.kind;
  advance();

  std::unique_ptr<ExprAST> right = unary();

  return std::make_unique<BinaryExprAST>(std::move(left), std::move(right), operatorKind);
}

std::unique_ptr<ExprAST> Parser::literal() {
  switch (currentToken.kind) {
    case TokenKind::NUMBER: {
      std::cout << "number" << std::endl;
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
    default: {
      Error::error(currentToken, "Expected literal", lexer);
      return nullptr;
    }
  }
}

std::unique_ptr<ExprAST> Parser::identifier() {
  Lexer::Token value = currentToken;
  advance();
  return std::make_unique<LiteralExprAST>(value);
}

std::unique_ptr<ExprAST> Parser::grouping() {
  consume(TokenKind::LEFT_PAREN, "Expected '(' before expression");
  std::unique_ptr<ExprAST> expr = expression(0);
  consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
  return expr;
}

std::unique_ptr<ExprAST> Parser::assignment() {
  std::unique_ptr<ExprAST> left = identifier();
  consume(TokenKind::EQUAL, "Expected '=' after identifier");
  std::unique_ptr<ExprAST> right = expression(0);
  return std::make_unique<BinaryExprAST>(std::move(left), std::move(right), TokenKind::EQUAL);
}
