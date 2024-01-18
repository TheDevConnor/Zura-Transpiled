#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

std::unique_ptr<StmtAST> Parser::declaration() {
  if (match(TokenKind::VAR))
    return std::unique_ptr<StmtAST>(varDeclaration());
  if (match(TokenKind::FUN))
    return std::unique_ptr<StmtAST>(functionDeclaration());
  return std::unique_ptr<StmtAST>(statement());
}

std::unique_ptr<StmtAST> Parser::statement() {
  if (match(TokenKind::PRINT))
    return std::unique_ptr<StmtAST>(printStatement());
  if (match(TokenKind::EXIT))
    return std::unique_ptr<StmtAST>(exitStatement());
  if (match(TokenKind::LEFT_BRACE))
    return std::unique_ptr<StmtAST>(blockStatement());
  if (match(TokenKind::RETURN))
    return std::unique_ptr<StmtAST>(returnStatement());
  return std::unique_ptr<StmtAST>(expressionStatement());
}

std::unique_ptr<StmtAST> Parser::blockStatement() {
  std::vector<std::unique_ptr<StmtAST>> statements;

  while (!match(TokenKind::RIGHT_BRACE)) {
    std::unique_ptr<StmtAST> stmt = declaration();
    statements.push_back(std::move(stmt));
  }

  return std::make_unique<BlockStmtAST>(std::move(statements));
}

std::unique_ptr<StmtAST> Parser::expressionStatement() {
  std::unique_ptr<ExprAST> expr = expression(0);
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return std::make_unique<ExpressionStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::returnStatement() {
  std::unique_ptr<ExprAST> expr = expression(0);
  consume(TokenKind::SEMICOLON, "Expected ';' after expression");
  return std::make_unique<ReturnStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::printStatement() {
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
  // Get the exit code
  if (currentToken.kind == TokenKind::NUMBER) {
    std::unique_ptr<ExprAST> expr = expression(0);
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExitStmtAST>(std::move(expr));
  } else if (match(TokenKind::IDENTIFIER)) {
    Lexer::Token ident = previousToken;
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExitStmtAST>(std::make_unique<IdentifierExprAST>(ident));
  } else
    Error::error(currentToken,
                       "Expected number or identifier after 'exit'", lexer);

  return nullptr;
}

// AstNode *Parser::functionDeclaration() {
//   consume(TokenKind::IDENTIFIER, "Expected function name");
//   Lexer::Token name = previousToken;

//   consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");

//   std::vector<Lexer::Token> parameters;
//   std::vector<AstNode *> paramType;
//   AstNode *pType = nullptr;

//   if (!match(TokenKind::RIGHT_PAREN)) {
//     do {
//       consume(TokenKind::IDENTIFIER, "Expected parameter name");
//       parameters.push_back(previousToken);

//       consume(TokenKind::COLON,
//               "Expected ':' after param name for type annotation");
//       // check expression for an array type annotation
//       if (match(TokenKind::LEFT_BRACKET)) {
//         pType = expression();
//       } else {
//         pType = findType(pType);
//       }
//       paramType.push_back(findType(pType));
//       advance();

//     } while (match(TokenKind::COMMA));
//     consume(TokenKind::RIGHT_PAREN, "Expected ')' after function parameters");
//   }

//   // The type to be returned '<type>'
//   AstNode *type = nullptr;
//   type = (currentToken.kind == TokenKind::RIGHT_BRACKET) ? expression() : findType(type);
//   if (type == nullptr)
//     Error::error(currentToken, "Expected return type for the function.", lexer);
//   advance();

//   consume(TokenKind::LEFT_BRACE, "Expected '{' before function body");
//   AstNode *body = blockStatement();

//   return new AstNode(AstNodeType::FUNCTION_DECLARATION,
//                      new AstNode::FunctionDeclaration(name, parameters,
//                                                       paramType, type, body));
// }

// AstNode *Parser::varDeclaration() {
//   consume(TokenKind::IDENTIFIER, "Expected variable name");

//   Lexer::Token name = previousToken;

//   AstNode *type = nullptr;
//   if (match(TokenKind::COLON)) {
//     if (match(TokenKind::LEFT_BRACKET)) {
//       type = expression();
//     } else {
//       type = findType(type);
//     }
//     advance();
//   }

//   AstNode *initializer = nullptr;
//   if (match(TokenKind::EQUAL))
//     initializer = expression();
//   else
//     Error::error(currentToken,
//                        "Expected '=' after variable type "
//                        "annotation or var name.",
//                        lexer);

//   consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
//   return new AstNode(AstNodeType::VAR_DECLARATION,
//                      new AstNode::VarDeclaration(name, type, initializer));
// }

std::unique_ptr<StmtAST> Parser::varDeclaration() {
  consume(TokenKind::IDENTIFIER, "Expected variable name");

  Lexer::Token name = previousToken;

  std::unique_ptr<TypeAST> type = nullptr;
  if (match(TokenKind::COLON)) {
    if (match(TokenKind::LEFT_BRACKET)) {
      type = std::make_unique<TypeAST>(expression);
    } else {
      type = std::make_unique<TypeAST>(findType(type.get()));
    }
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

  consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
  return std::make_unique<VarDeclStmtAST>(name, std::move(type), std::move(initializer));
}
