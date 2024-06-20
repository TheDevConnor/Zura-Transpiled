#include "../ast/expr.hpp"
#include "parser.hpp"

#include <iostream>
#include <sstream>

Node::Expr *Parser::parseExpr(PStruct *psr, BindingPower bp) {
    auto left = nud(psr);

    while (bp < getBP(psr->advance(psr).kind)) {
        left = led(psr, left, bp);
    }

    return left;
}

Node::Expr *Parser::primary(PStruct* psr) {
    std::unordered_map<TokenKind, Node::Expr *> primaryMap = {
        { TokenKind::NUMBER, new NumberExpr(psr->current(psr).value) },
        { TokenKind::IDENTIFIER, new IdentExpr(psr->current(psr).value) },
        { TokenKind::STRING, new StringExpr(psr->current(psr).value) }
    };
    return primaryMap[psr->current(psr).kind];
}

Node::Expr *Parser::group(PStruct *psr) {
    return nullptr;
}

Node::Expr *Parser::unary(PStruct *psr) {
    auto op = psr->current(psr);
    psr->advance(psr);
    auto right = parseExpr(psr, defaultValue);

    return new UnaryExpr(
        right,
        op.value
    );
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
    auto op = psr->current(psr);
    psr->advance(psr);
    auto right = parseExpr(psr, defaultValue);
    return new BinaryExpr(
        left,
        right,
        op.value
    );
}