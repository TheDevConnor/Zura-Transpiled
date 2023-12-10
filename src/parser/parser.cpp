#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Parser::Parser(const char* source) : source(source) {}

AstNode* Parser::parse() {
    lexer.initToken(source);

    advance();

    AstNode* expr = expression();
    consume(TokenKind::END_OF_FILE, "Expected end of expression");

    expr->printAst(expr, 0);

    return expr;
}

AstNode* Parser::expression(int precedence) {
    AstNode* left = unary();
    return binary(left, precedence);
}

AstNode* Parser::unary() {
    if (match(TokenKind::MINUS) || match(TokenKind::BANG)) {
        TokenKind op = previousToken.kind;
        AstNode* right = unary();

        return new AstNode(AstNodeType::UNARY, new AstNode::Unary(op, right));
    }

    return grouping();
}

AstNode* Parser::binary(AstNode *left, int precedence) {
    while (true) {
        int currentPrecedence = getPrecedence();

        if (currentPrecedence < precedence) return left;

        TokenKind op = currentToken.kind;
        advance();

        AstNode* right = unary();
        left = new AstNode(AstNodeType::BINARY, new AstNode::Binary(left, op, right));
    }
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
    if (match(TokenKind::NUMBER) || match(TokenKind::STRING)) {
        return new AstNode(AstNodeType::LITERAL, new AstNode::Literal(currentToken));
    }

    /// Handle other literals here...

    /// If none of the literal patterns match
    synchronize();
    return nullptr;
}