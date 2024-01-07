#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

std::vector<AstNode *> Parser::lookupMain() {
  std::vector<AstNode *> statements; // Statements in main function
  AstNode *main = nullptr;           // Main function

  while (!match(TokenKind::END_OF_FILE)) {
    AstNode *stmt = declaration();
    statements.push_back(stmt);
  }

  for (AstNode *stmt : statements) {
    if (stmt->type == AstNodeType::FUNCTION_DECLARATION) {
      AstNode::FunctionDeclaration *function =
          (AstNode::FunctionDeclaration *)stmt->data;
      if (function->name.start[0] == 'm' && function->name.start[1] == 'a' &&
          function->name.start[2] == 'i' && function->name.start[3] == 'n') {
        main = stmt;
        break;
      }
    }
  }

  if (!main)
    Error::error(previousToken,
                       "No main function found. Please include a main function",
                       lexer);

  return statements;
}

bool Parser::match(TokenKind kinds) {
  if (currentToken.kind != kinds)
    return false;

  advance();
  return true;
}

void Parser::advance() {
  previousToken = currentToken;

  while (true) {
    currentToken = lexer.scanToken();
    if (currentToken.kind != TokenKind::ERROR_)
      break;

    hadError = true;
    Error::error(currentToken, "Unexpected character", lexer);
  }
}

Lexer::Token Parser::peek(int offset) {
  Lexer::Token token = currentToken;
  for (int i = 0; i < offset; i++)
    advance();
  Lexer::Token peeked = currentToken;
  currentToken = token;
  return peeked;
}

void Parser::consume(TokenKind kind, std::string message) {
  if (currentToken.kind == kind) {
    advance();
    return;
  }

  hadError = true;
  Error::error(currentToken, message, lexer);
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

int Parser::getPrecedence() {
  switch (currentToken.kind) {
  case TokenKind::PLUS:
  case TokenKind::MINUS:
    return 1;
  case TokenKind::STAR:
  case TokenKind::SLASH:
  case TokenKind::MODULO:
    return 2;
  default:
    return -1;
  }
}

AstNode *Parser::findType(AstNode *type) {
  switch (currentToken.kind) {
  case I8:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case I16:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case I32:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case I64:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case U8:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case U16:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case U32:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case U64:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case F32:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case F64:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case Bool:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  case STRING:
    type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken));
    break;
  default:
    consume(currentToken.kind,
            "Expected type annotation of either i8, i16, i32, i64, u8, u16, "
            "u32, u64, f32, f64, bool or string");
  }

  return type;
}
