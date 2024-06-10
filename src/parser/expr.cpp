#include "../ast/expr.hpp"
#include "parser.hpp"
#include "map.hpp"

#include <iostream>

using namespace ParserNamespace;

Node::Expr *ParserNamespace::parseExpr(Parser *psr, BindingPower bp) {
    auto left = nudHandler(psr, psr->current(psr).kind);
    
    while (bp < getBP(psr, psr->current(psr).kind)) { 
       left = ledHandler(psr, left);
       psr->advance(psr);
    }

    return left; 
}

Node::Expr *ParserNamespace::num(Parser *psr) {
    auto res = psr->current(psr).value;
    psr->advance(psr);
    return new NumberExpr(res);
}

Node::Expr *ParserNamespace::ident(Parser *psr) {
    auto res = psr->current(psr).value;
    psr->advance(psr);
    return new IdentExpr(res);
}

Node::Expr *ParserNamespace::str(Parser *psr) {
    auto res = psr->current(psr).value;
    psr->advance(psr);
    return new StringExpr(res);
}

Node::Expr *ParserNamespace::unary(Parser *psr) {
    auto op = psr->current(psr).value;
    psr->advance(psr);
    auto right = parseExpr(psr, BindingPower::prefix);
    return new UnaryExpr(right, op);
}

Node::Expr *ParserNamespace::binary(Parser *psr, Node::Expr *left, std::string op, 
                                BindingPower bp) {
    auto right = parseExpr(psr, bp);
    return new BinaryExpr(left, right, op);
}