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

        switch (op) {
            case TokenKind::PLUS: {
                if (left->type == AstNodeType::STRING_LITERAL && right->type == AstNodeType::NUMBER_LITERAL) {
                    ParserError::error(currentToken, "Cannot add a string to a number", lexer);
                } else if (left->type == AstNodeType::NUMBER_LITERAL && right->type == AstNodeType::STRING_LITERAL) {
                    ParserError::error(currentToken, "Cannot add a number to a string", lexer);
                }
                break;
            }
            case TokenKind::MINUS:
            case TokenKind::STAR:
            case TokenKind::SLASH: {
                if (left->type == AstNodeType::STRING_LITERAL || right->type == AstNodeType::STRING_LITERAL) {
                    ParserError::error(currentToken, "Cannot subtract, multiply, or divide a string", lexer);
                }
                break;
            }
            default:
                break;
        }
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
    switch (currentToken.kind) {
        case TokenKind::NUMBER: {
            double value = std::stod(currentToken.start);
            advance();
            return new AstNode(AstNodeType::NUMBER_LITERAL, new AstNode::NumberLiteral(value));
        }
        case TokenKind::STRING: {
            std::string value = currentToken.start;
            advance();
            return new AstNode(AstNodeType::STRING_LITERAL, new AstNode::StringLiteral(value));
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
        default:
            ParserError::error(currentToken, "Expected literal", lexer);
    }

    /// If none of the literal patterns match
    synchronize();
    return nullptr;
}