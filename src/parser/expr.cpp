#include "../ast/ast.hpp"
#include "../helper/error/parserError.hpp"
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

AstNode *Parser::expression(int precedence) {
  AstNode *left = unary();

  // function call
  if (match(TokenKind::LEFT_PAREN)) {
    std::vector<AstNode *> arguments;

    if (!match(TokenKind::RIGHT_PAREN)) {
      do {
        if (arguments.size() >= 255)
          ParserError::error(currentToken,
                             "Cannot have more than 255 arguments", lexer);
        arguments.push_back(expression());
      } while (match(TokenKind::COMMA));
    }

    consume(TokenKind::RIGHT_PAREN, "Expected ')' after arguments");

    return new AstNode(AstNodeType::CALL, new AstNode::Call(left, arguments));
  }

  return binary(left, precedence);
}

AstNode *Parser::unary() {
  if (match(TokenKind::MINUS) || match(TokenKind::BANG)) {
    TokenKind op = previousToken.kind;
    AstNode *right = unary();

    return new AstNode(AstNodeType::UNARY, new AstNode::Unary(op, right));
  }

  return grouping();
}

AstNode *Parser::binary(AstNode *left, int precedence) {
  while (true) {
    int currentPrecedence = getPrecedence();

    if (currentPrecedence < precedence)
      return left;

    TokenKind op = currentToken.kind;
    advance();

    AstNode *right = unary();

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
    case TokenKind::MODULO:
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
    left =
        new AstNode(AstNodeType::BINARY, new AstNode::Binary(left, op, right));
  }
}

AstNode *Parser::grouping() {
  if (match({TokenKind::LEFT_PAREN})) {
    AstNode *expr = expression();
    consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
    return new AstNode(AstNodeType::GROUPING, new AstNode::Grouping(expr));
  }

  return literal();
}

AstNode *Parser::literal() {
  switch (currentToken.kind) {
  case TokenKind::IDENTIFIER: {
    Lexer::Token value = currentToken;
    advance();
    return new AstNode(AstNodeType::IDENTIFIER, new AstNode::Identifier(value));
  }
  case TokenKind::NUMBER: {
    double value = std::stod(currentToken.start);
    advance();
    return new AstNode(AstNodeType::NUMBER_LITERAL,
                       new AstNode::NumberLiteral(value));
  }
  case TokenKind::STRING: {
    std::string value = currentToken.start;
    advance();
    return new AstNode(AstNodeType::STRING_LITERAL,
                       new AstNode::StringLiteral(value));
  }
  case TokenKind::TR: {
    advance();
    return new AstNode(AstNodeType::TRUE_LITERAL, nullptr);
  }
  case TokenKind::FAL: {
    advance();
    return new AstNode(AstNodeType::FALSE_LITERAL, nullptr);
  }
  case TokenKind::NIL: {
    advance();
    return new AstNode(AstNodeType::NIL_LITERAL, nullptr);
  }
  case TokenKind::RIGHT_BRACKET: {
    std::cout << "Array type annotation" << std::endl;
    advance();
    AstNode *type = nullptr;
    type = findType(type);
    return new AstNode(AstNodeType::ARRAY_TYPE, new AstNode::ArrayType(type));
  }
  case TokenKind::LEFT_BRACKET: {
    // array type annotation
    advance();
    std::vector<AstNode *> elements;
    AstNode *type = nullptr;

    if (!match(TokenKind::RIGHT_BRACKET)) {
      do {
        if (elements.size() >= 255)
          ParserError::error(currentToken,
                             "Cannot have more than 255 elements", lexer);
        elements.push_back(expression());
      } while (match(TokenKind::COMMA));
    }

    consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array elements");

    return new AstNode(AstNodeType::ARRAY, new AstNode::Array(elements));
  }
  default:
    ParserError::error(currentToken, "Expected literal", lexer);
  }

  /// If none of the literal patterns match
  synchronize();
  return nullptr;
}
