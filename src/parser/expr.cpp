#include "../ast/expr.hpp"
#include "parser.hpp"

#include <iostream>

Node::Expr *Parser::parseExpr(PStruct *psr, BindingPower bp) {
    auto *left = nud(psr);

    while (getBP(psr->current(psr).kind) > bp) {
       left = led(psr, left, getBP(psr->current(psr).kind));
    }

    return left;
}

Node::Expr *Parser::primary(PStruct* psr) {
    switch (psr->current(psr).kind) {
    case TokenKind::NUMBER: {
        return new NumberExpr(std::stod(psr->advance(psr).value));
    }
    case TokenKind::IDENTIFIER: {
        return new IdentExpr(psr->advance(psr).value);
    }
    case TokenKind::STRING: {
        return new StringExpr(psr->advance(psr).value);
    }
    default:
        std::cout << "Could not parse primary expression" << std::endl;
        return nullptr;
    }
}

Node::Expr *Parser::group(PStruct *psr) {
    psr->expect(psr, TokenKind::LEFT_PAREN);
    auto *expr = parseExpr(psr, defaultValue);
    psr->expect(psr, TokenKind::RIGHT_PAREN);
    return new GroupExpr(expr);
}

Node::Expr *Parser::unary(PStruct *psr) {
    auto op = psr->advance(psr);
    auto *right = parseExpr(psr, defaultValue);

    return new UnaryExpr(
        right,
        op.value
    );
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
    auto op = psr->advance(psr);
    auto *right = parseExpr(psr, bp);

    return new BinaryExpr(
        left,
        right,
        op.value
    );
}

Node::Expr *Parser::assign(PStruct *psr, Node::Expr *left, BindingPower bp) {
    auto op = psr->advance(psr);
    auto *right = parseExpr(psr, defaultValue);

    return new AssignmentExpr(
        left,
        op.value,
        right 
    );
}