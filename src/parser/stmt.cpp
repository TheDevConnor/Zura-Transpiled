#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

std::unique_ptr<StmtAST> Parser::declaration() {
  if (match(TokenKind::VAR))
    return varDeclaration();
  if (match(TokenKind::FUN))
    return functionDeclaration();
  return statement();
}

std::unique_ptr<StmtAST> Parser::statement() {
  if (match(TokenKind::PRINT))
    return printStatement();
  if (match(TokenKind::EXIT))
    return exitStatement();
  if (match(TokenKind::LEFT_BRACE))
    return blockStatement();
  if (match(TokenKind::RETURN))
    return returnStatement();
  return expressionStatement();
}

std::unique_ptr<StmtAST> Parser::blockStatement() {
  std::vector<std::unique_ptr<StmtAST>> statements;

  while (!match(TokenKind::RIGHT_BRACE)) {
    std::cout << "blockStatement" << std::endl;
    std::unique_ptr<StmtAST> stmt = declaration();
    statements.push_back(std::move(stmt));
  }

  return std::make_unique<BlockStmtAST>(std::move(statements));
}

std::unique_ptr<StmtAST> Parser::expressionStatement() {
  std::cout << "expressionStatement" << std::endl;
  std::unique_ptr<ExprAST> expr = expression(0);
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return std::make_unique<ExpressionStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::returnStatement() {
  std::cout << "returnStatement" << std::endl;
  std::unique_ptr<ExprAST> expr = expression(0);
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return std::make_unique<ReturnStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::printStatement() {
  std::cout << "printStatement" << std::endl;
  std::unique_ptr<ExprAST> expr = expression(0);
  std::vector<std::unique_ptr<ExprAST>> idents;

  // check if there are multiple commas in the print statement
  if (match(TokenKind::COMMA)) {
    std::vector<std::unique_ptr<ExprAST>> idents;
    std::unique_ptr<ExprAST> exprs = expression(0);
    idents.push_back(std::move(exprs));
    while (peek(0).kind != TokenKind::SEMICOLON) {
      consume(TokenKind::COMMA, "Expected ',' after expression");
      std::unique_ptr<ExprAST> exprs = expression(0);
      idents.push_back(std::move(exprs));
    }
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<PrintStmtAST>(std::move(expr), std::move(idents));
  }

  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return std::make_unique<PrintStmtAST>(std::move(expr), std::move(idents));
}

std::unique_ptr<StmtAST> Parser::exitStatement() {
  std::cout << "exitStatement" << std::endl;
  if (currentToken.kind == TokenKind::NUMBER) {
    std::unique_ptr<ExprAST> expr = expression(0);
    // consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExitStmtAST>(std::move(expr));
  } else {
    // consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExitStmtAST>(nullptr);
  }
}

std::unique_ptr<StmtAST> Parser::functionDeclaration() {
  std::cout << "functionDeclaration" << std::endl;
  consume(TokenKind::IDENTIFIER, "Expected function name");

  std::string name = previousToken.start;

  consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");

  std::vector<std::string> params;
  std::vector<TypeAST*> paramTypes;
  if (!match(TokenKind::RIGHT_PAREN)) {
    do {
      consume(TokenKind::IDENTIFIER, "Expected parameter name");
      consume(TokenKind::COLON, "Expected ':' after parameter name");
      
      std::string type = findType(checkType());
      TypeAST *paramType = buildType(type);
      paramTypes.push_back(paramType);

      consume(TokenKind::IDENTIFIER, "Expected parameter name");
      std::string param = previousToken.start;
      params.push_back(param);
    } while (match(TokenKind::COMMA));
  }

  std::string returnType = findType(checkType());
  TypeAST *resultType = buildType(returnType);
  advance();

  consume(TokenKind::LEFT_BRACE, "Expected '{' before function body");

  std::unique_ptr<StmtAST> body = blockStatement();

  return std::make_unique<FunctionDeclStmtAST>(name, std::move(params), std::move(paramTypes), std::move(resultType),
                                               std::move(body));
}

std::unique_ptr<StmtAST> Parser::varDeclaration() {
  consume(TokenKind::IDENTIFIER, "Expected variable name");

  Lexer::Token name = previousToken;

  std::unique_ptr<TypeAST> type = nullptr;
  if (match(TokenKind::COLON)) {
    std::string typeName = findType(checkType());
    type = std::make_unique<TypeAST>(typeName);
    advance();
  }

  std::unique_ptr<ExprAST> initializer = nullptr;
  if (match(TokenKind::EQUAL))
    initializer = expression(0);
  else
    Error::error(currentToken,
                 "Expected '=' after variable type "
                 "annotation or var name.",
                 lexer);

  // consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
  return std::make_unique<VarDeclStmtAST>(name, std::move(type),
                                          std::move(initializer));
}
