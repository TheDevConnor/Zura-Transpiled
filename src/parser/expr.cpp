#include "../ast/ast.hpp"
#include "../helper/error/parserError.hpp"
#include "parser.hpp"
#include <memory>

std::unique_ptr<AstNode> Parser::expression(int precedence) {
  if (match(TokenKind::IDENTIFIER)) {
    Lexer::Token identifier = previousToken;
    return std::make_unique<AstNode>(AstNodeType::IDENTIFIER,
                                     new AstNode::Identifier(identifier));
  }
  std::unique_ptr<AstNode> left = unary();
  return binary(std::move(left), 0);
}

std::unique_ptr<AstNode> Parser::unary() {
  if (match(TokenKind::MINUS) || match(TokenKind::BANG)) {
    TokenKind op = previousToken.kind;
    std::unique_ptr<AstNode> right = unary();

    return std::make_unique<AstNode>(AstNodeType::UNARY,
                                     new AstNode::Unary(op, std::move(right)));
  }

  return grouping();
}

std::unique_ptr<AstNode> Parser::binary(std::unique_ptr<AstNode> left,
                                        int precedence) {
  while (true) {
    int currentPrecedence = getPrecedence();

    if (currentPrecedence < precedence)
      return left;

    TokenKind op = currentToken.kind;
    advance();

    std::unique_ptr<AstNode> right = unary();

    switch (op) {
    case TokenKind::PLUS: {
      if (left->type == AstNodeType::STRING_LITERAL &&
          right->type == AstNodeType::NUMBER_LITERAL) {
        ParserError::error(currentToken, "Cannot add a string to a number",
                           lexer);
      } else if (left->type == AstNodeType::NUMBER_LITERAL &&
                 right->type == AstNodeType::STRING_LITERAL) {
        ParserError::error(currentToken, "Cannot add a number to a string",
                           lexer);
      }
      break;
    }
    case TokenKind::MINUS:
    case TokenKind::STAR:
    case TokenKind::SLASH: {
      if (left->type == AstNodeType::STRING_LITERAL ||
          right->type == AstNodeType::STRING_LITERAL) {
        ParserError::error(currentToken,
                           "Cannot subtract, multiply, or divide a string",
                           lexer);
      }
      break;
    }
    default:
      break;
    }
    left = std::make_unique<AstNode>(AstNodeType::BINARY,
                                     new AstNode::Binary(std::move(left), op, std::move(right)));
  }
}

std::unique_ptr<AstNode> Parser::grouping() {
  if (match(TokenKind::LEFT_PAREN)) {
    std::unique_ptr<AstNode> expr = expression();
    consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
    return std::make_unique<AstNode>(AstNodeType::GROUPING,
                                     new AstNode::Grouping(std::move(expr)));
  }

  return literal();
}

std::unique_ptr<AstNode> Parser::literal() {
  switch (currentToken.kind) {
  case TokenKind::NUMBER: {
    double value = std::stod(currentToken.start);
    advance();
    return std::make_unique<AstNode>(AstNodeType::NUMBER_LITERAL,
                                     new AstNode::NumberLiteral(value));
  }
  case TokenKind::STRING: {
    std::string value = currentToken.start;
    advance();
    return std::make_unique<AstNode>(AstNodeType::STRING_LITERAL,
                                     new AstNode::StringLiteral(value));
  }
  case TokenKind::TR: {
    advance();
    return std::make_unique<AstNode>(AstNodeType::TRUE_LITERAL, nullptr);
  }
  case TokenKind::FAL: {
    advance();
    return std::make_unique<AstNode>(AstNodeType::FALSE_LITERAL, nullptr);
  }
  case TokenKind::NIL: {
    advance();
    return std::make_unique<AstNode>(AstNodeType::NIL_LITERAL, nullptr);
  }
  default:
    ParserError::error(currentToken, "Expected literal", lexer);
  }

  /// If none of the literal patterns match
  synchronize();
  return nullptr;
}
