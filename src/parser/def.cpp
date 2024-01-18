#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

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

TypeAST *Parser::findType(TypeAST *type) {
  return type;
}

std::vector<std::unique_ptr<StmtAST>> Parser::lookupMain() {
  std::vector<std::unique_ptr<StmtAST>> statements;

  while (currentToken.kind != TokenKind::END_OF_FILE) {
    statements.push_back(std::move(declaration()));
  }

  bool foundMain = false;
  for (size_t i = 0; i < statements.size(); i++) {
    std::unique_ptr<StmtAST>& stmt = statements[i];
    
    if (stmt.get()->getNodeType() == AstNodeType::FUNCTION_DECLARATION) {
      FunctionDeclStmtAST* funcDecl = (FunctionDeclStmtAST*)stmt.get();
      if (funcDecl->Name == "main") {
        foundMain = true;
        break;
      }
    }

    if (i == statements.size() - 1) {
      hadError = true;
      Error::error(currentToken, "No main function found", lexer);
    }
  }

  return statements;
}
