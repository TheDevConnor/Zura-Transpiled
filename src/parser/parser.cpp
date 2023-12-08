#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Parser::Parser(const char* source) : source(source) {}

AstNode* Parser::parse() {
    lexer.initToken(source);
    currentToken = lexer.scanToken();

    AstNode* expr = expression();
    consume(TokenKind::END_OF_FILE, "Expected end of expression");

    expr->printAst(expr, 0);

    return expr;
}

AstNode* Parser::expression(int precedence) {
    AstNode* left = unary();

    while (getPrecedence() >= precedence) {
        TokenKind op = currentToken.kind;
        consume(op, "Expected binary operator");

        AstNode* right = expression(getPrecedence() + 1);

        left = new AstNode(AstNodeType::BINARY, new AstNode::Binary(left, op, right));
    }

    return left;
}

AstNode* Parser::unary() {
    if (check({TokenKind::MINUS, TokenKind::BANG})) {
        TokenKind op = currentToken.kind;
        consume(op, "Expected unary operator");
        AstNode* right = unary();
        return new AstNode(AstNodeType::UNARY, new AstNode::Unary(op, right));
    }

    return grouping();
}

AstNode* Parser::grouping() {
    if (match({TokenKind::LEFT_PAREN})) {
        AstNode* expr = expression();
        consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
        return new AstNode(AstNodeType::GROUPING, new AstNode::Grouping(expr));
    }

    return literal();
}

AstNode* Parser::literal() {
    if (match({TokenKind::NUMBER, TokenKind::STRING})) {
        return new AstNode(AstNodeType::LITERAL, new AstNode::Literal(currentToken));
    }

    /// Handle other literals here...

    /// If none of the literal patterns match
    synchronize();
    return nullptr;
}

bool Parser::match(std::vector<TokenKind> kinds) {
    for (TokenKind kind : kinds) {
        if (check({kind})) {
            consume(kind, ""); // consume the matched token
            return true;
        }
    }

    return false;
}

bool Parser::check(std::vector<TokenKind> kinds) {
    for (const auto& kind : kinds) {
        if (currentToken.kind == kind) return true;
    }

    return false;
}

Lexer::Token Parser::consume(TokenKind kind, std::string message) {
    if (check({kind})) {
        Lexer::Token token = currentToken;
        currentToken = lexer.scanToken();
        return token;
    }

    ParserError::error(currentToken, currentToken.column, message, lexer);
    return lexer.errorToken(message);
}

void Parser::synchronize() {
    currentToken = lexer.scanToken();

    while (currentToken.kind != TokenKind::END_OF_FILE) {
        if (currentToken.kind == TokenKind::SEMICOLON) return;

        switch (currentToken.kind) {
            case TokenKind::CLASS:
            case TokenKind::FUN:
            case TokenKind::VAR:
            case TokenKind::LOOP:
            case TokenKind::IF:
            case TokenKind::PRINT:
            case TokenKind::RETURN:
                return;
            default:
                break;
        }

        currentToken = lexer.scanToken();
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