#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

AstNode* Parser::declaration() {
    if (match(TokenKind::VAR)) return varDeclaration();
    if (match(TokenKind::FUN)) return functionDeclaration();
    return statement();
}

AstNode* Parser::statement() {
    if (match(TokenKind::PRINT)) return printStatement();
    if (match(TokenKind::LEFT_BRACE)) return blockStatement();
    return expressionStatement();
}

AstNode* Parser::blockStatement() {
    std::vector<AstNode*> statements;

    while (!match(TokenKind::RIGHT_BRACE)) {
        AstNode* stmt = declaration();
        statements.push_back(stmt);
    }

    return new AstNode(AstNodeType::BLOCK, new AstNode::Block(statements));
}

AstNode* Parser::expressionStatement() {
    AstNode* expr = expression();
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::EXPRESSION, new AstNode::Stmt{ AstNode::Expression(expr) });
}

AstNode* Parser::printStatement() {
    AstNode* expr = expression();

    consume(TokenKind::COMMA, "Expected ',' after expression");
    consume(TokenKind::IDENTIFIER, "Expected identifier after ','");
    Lexer::Token ident = previousToken;
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");

    return new AstNode(AstNodeType::PRINT, new AstNode::Print(expr, ident));
}

AstNode* Parser::functionDeclaration() {
    consume(TokenKind::IDENTIFIER, "Expected function name");
    Lexer::Token name = previousToken;

    consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");

    std::vector<Lexer::Token> parameters;

    if (!match(TokenKind::RIGHT_PAREN)) {
        do {
            consume(TokenKind::IDENTIFIER, "Expected parameter name");
            parameters.push_back(previousToken);
        } while (match(TokenKind::COMMA));
        consume(TokenKind::RIGHT_PAREN, "Expected ')' after function parameters");
    }

    // The type to be returned '-> type'
    AstNode* type = nullptr;
    if (match(TokenKind::MINUS) && match(TokenKind::GREATER)) {
        type = findType(type);
        advance();
    } else ParserError::error(currentToken, "Expected '->' followed by return type annotation", lexer);

    consume(TokenKind::LEFT_BRACE, "Expected '{' before function body");
    AstNode* body = blockStatement();

    return new AstNode(AstNodeType::FUNCTION_DECLARATION, new AstNode::FunctionDeclaration(name, parameters, type, body));
}

AstNode* Parser::varDeclaration() {
    consume(TokenKind::IDENTIFIER, "Expected variable name");

    Lexer::Token name = previousToken;

    AstNode* type = nullptr;
    if (match(TokenKind::LESS)) {
        type = findType(type);
        advance();
        consume(TokenKind::GREATER, "Expected '>' after type annotation");
    }

    AstNode* initializer = nullptr;
    if (match(TokenKind::COLON) && match(TokenKind::EQUAL)) initializer = expression(); // := because the lexer not wont to work
    else ParserError::error(currentToken, "Expected ':' followed by '=' after variable type annotation or var name.", lexer);

    consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
    return new AstNode(AstNodeType::VAR_DECLARATION, new AstNode::VarDeclaration(name, type, initializer));
}

