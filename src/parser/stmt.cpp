#include "../ast/ast.hpp"
#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

AstNode *Parser::declaration() {
  if (match(TokenKind::VAR))
    return varDeclaration();
  if (match(TokenKind::FUN))
    return functionDeclaration();
  return statement();
}

AstNode *Parser::statement() {
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

AstNode *Parser::blockStatement() {
  std::vector<AstNode *> statements;

  while (!match(TokenKind::RIGHT_BRACE)) {
    AstNode *stmt = declaration();
    statements.push_back(stmt);
  }

  return new AstNode(AstNodeType::BLOCK, new AstNode::Block(statements));
}

AstNode *Parser::expressionStatement() {
  AstNode *expr = expression();
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return new AstNode(AstNodeType::EXPRESSION,
                     new AstNode::Stmt{AstNode::Expression(expr)});
}

AstNode *Parser::returnStatement() {
  AstNode *expr = expression();
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return new AstNode(AstNodeType::RETURN, new AstNode::Return(expr));
}

AstNode *Parser::printStatement() {
  AstNode *expr = expression();
  std::vector<AstNode *> ident;

  // check if there are multiple commas in the print statement
  if (match(TokenKind::COMMA)) {
    std::vector<AstNode *> idents;
    AstNode *exprs = expression();
    idents.push_back(exprs);
    while (peek(0).kind != TokenKind::SEMICOLON) {
      consume(TokenKind::COMMA, "Expected ',' after expression");
      AstNode *exprs = expression();
      idents.push_back(exprs);
    }
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::PRINT, new AstNode::Print(expr, idents));
  }

  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return new AstNode(AstNodeType::PRINT, new AstNode::Print(expr, ident));
}

AstNode *Parser::exitStatement() {
  // Get the exit code
  if (currentToken.kind == TokenKind::NUMBER) {
    AstNode *expr = expression();
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::EXIT, new AstNode::Exit(expr));
  } else if (match(TokenKind::IDENTIFIER)) {
    Lexer::Token ident = previousToken;
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::EXIT, new AstNode::Exit(new AstNode(
                                              AstNodeType::IDENTIFIER,
                                              new AstNode::Identifier(ident))));
  } else
    ParserError::error(currentToken,
                       "Expected number or identifier after 'exit'", lexer);

  return nullptr;
}

AstNode *Parser::functionDeclaration() {
  consume(TokenKind::IDENTIFIER, "Expected function name");
  Lexer::Token name = previousToken;

  consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");

  std::vector<Lexer::Token> parameters;
  std::vector<AstNode *> paramType;
  AstNode *pType = nullptr;

  if (!match(TokenKind::RIGHT_PAREN)) {
    do {
      consume(TokenKind::IDENTIFIER, "Expected parameter name");
      parameters.push_back(previousToken);

      consume(TokenKind::COLON,
              "Expected ':' after param name for type annotation");
      paramType.push_back(findType(pType));
      advance();

    } while (match(TokenKind::COMMA));
    consume(TokenKind::RIGHT_PAREN, "Expected ')' after function parameters");
  }

  // The type to be returned '<type>'
  AstNode *type = nullptr;
  if (match(TokenKind::LESS)) {
    type = findType(type);
    advance();
  } else
    ParserError::error(
        currentToken, "Expected '<' followed by return type annotation", lexer);
  consume(TokenKind::GREATER, "Expected '>' after type annotation");
  consume(TokenKind::LEFT_BRACE, "Expected '{' before function body");
  AstNode *body = blockStatement();

  return new AstNode(AstNodeType::FUNCTION_DECLARATION,
                     new AstNode::FunctionDeclaration(name, parameters,
                                                      paramType, type, body));
}

AstNode *Parser::varDeclaration() {
  consume(TokenKind::IDENTIFIER, "Expected variable name");

  Lexer::Token name = previousToken;

  AstNode *type = nullptr;
  if (match(TokenKind::LESS)) {
    type = findType(type);
    advance();
    consume(TokenKind::GREATER, "Expected '>' after type annotation");
  }

  AstNode *initializer = nullptr;
  if (match(TokenKind::COLON) && match(TokenKind::EQUAL))
    initializer = expression(); // := because the lexer not wont to work
  else
    ParserError::error(currentToken,
                       "Expected ':' followed by '=' after variable type "
                       "annotation or var name.",
                       lexer);

  consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
  return new AstNode(AstNodeType::VAR_DECLARATION,
                     new AstNode::VarDeclaration(name, type, initializer));
}
