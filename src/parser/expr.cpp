#include "../ast/expr.hpp"
#include "parser.hpp"
#include "map.hpp"

#include <iostream>
#include <sstream>

using namespace ParserNamespace;

Node::Expr *ParserNamespace::parseExpr(Parser *psr, BindingPower bp) {
    auto left = nud_lu[psr->current(psr).kind](psr);
    while (bp < bp_lu[psr->current(psr).kind]) {
        left = led_lu[psr->current(psr).kind](psr, left, bp);
    }
    return left; 
}

Node::Expr *ParserNamespace::primary(Parser* psr) {
    std::unordered_map<TokenKind, Node::Expr *> primaryMap = {
        { TokenKind::NUMBER, new NumberExpr(psr->current(psr).value) },
        { TokenKind::IDENTIFIER, new IdentExpr(psr->current(psr).value) },
        { TokenKind::STRING, new StringExpr(psr->current(psr).value) }
    };
    return primaryMap[psr->current(psr).kind];
}

Node::Expr *ParserNamespace::group(Parser *psr) {
    return nullptr;
}

Node::Expr *ParserNamespace::unary(Parser *psr) {
    auto op = psr->current(psr);
    auto right = parseExpr(psr, defaultValue);

    return new UnaryExpr(
        right,
        op.value
    );
}

Node::Expr *ParserNamespace::binary(Parser *psr, Node::Expr *left, BindingPower bp) {
    auto op = psr->advance(psr);
    auto right = parseExpr(psr, defaultValue);
    return new BinaryExpr(
        left,
        right,
        op.value
    );
}