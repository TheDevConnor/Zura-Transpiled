#include "../ast/expr.hpp"
#include "parser.hpp"
#include "map.hpp"

#include <iostream>

using namespace ParserNamespace;

Node::Expr *ParserNamespace::parseExpr(Parser *psr, BindingPower bp) {
    
}

Node::Expr *ParserNamespace::primary(Parser* psr) {
    switch(psr->current(psr).kind) {
        case TokenKind::NUMBER:
            return new NumberExpr(psr->current(psr).value);
        case TokenKind::IDENTIFIER:
            return new IdentExpr(psr->current(psr).value);
        case TokenKind::STRING:
            return new StringExpr(psr->current(psr).value);
        default: {
            std::cout << "CANNONT PARSE PRIMARY EXPR! (" << psr->current(psr).value << ")\n";
            exit(0);
        }
    }
}

Node::Expr *ParserNamespace::group(Parser *psr) {
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