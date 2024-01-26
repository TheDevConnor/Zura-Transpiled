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

std::string Parser::findType(TokenKind kind) {
  std::unordered_map<TokenKind, std::string> types = {
      {TokenKind::I8, "i8"},   {TokenKind::I16, "i16"}, {TokenKind::I32, "i32"}, {TokenKind::I64, "i64"}, {TokenKind::I128, "i128"},
      {TokenKind::U8, "u8"},   {TokenKind::U16, "u16"}, {TokenKind::U32, "u32"}, {TokenKind::U64, "u64"}, {TokenKind::U128, "u128"},
      {TokenKind::F32, "f32"}, {TokenKind::F64, "f64"}, {TokenKind::STRING, "str"}, {TokenKind::Bool, "bool"}};

  return types[kind];
}

TypeAST *Parser::buildType(std::string type) {
  TypeAST *result = new TypeAST(type);
  return result;
}

std::vector<std::unique_ptr<StmtAST>> Parser::lookupMain() {
  std::vector<std::unique_ptr<StmtAST>> statements;
  StmtAST *main = nullptr;

  while (!match(TokenKind::END_OF_FILE)) {
    std::unique_ptr<StmtAST> stmt = declaration();
    statements.push_back(std::move(stmt));
  }

  // for (std::unique_ptr<StmtAST> &stmt : statements) {
  //   if (stmt->getNodeType() == AstNodeType::FUNCTION_DECLARATION) {
  //     FunctionDeclStmtAST *function =
  //         (FunctionDeclStmtAST *)stmt.get();
  //     if (function->Name[0] == 'm' && function->Name[1] == 'a' &&
  //         function->Name[2] == 'i' && function->Name[3] == 'n') {
  //       std::cout << "Found main function" << std::endl;
  //       main = stmt.get();
  //       break;
  //     }
  //   }
  // }
  // if (!main)
  //   Error::error(previousToken,
  //               "No main function found. Please include a main function",
  //               lexer);

  return statements;
}
