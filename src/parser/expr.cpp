#include "../ast/expr.hpp"
#include "parser.hpp"
#include "map.hpp"

Node::Expr *ParserClass::parseExpr(Parser *psr, BindingPower bp) {
   auto left = nudHandler(psr, psr->current(psr).kind);

   while (getBP(psr, psr->current(psr).kind) > bp) {
       left = ledHandler(psr, left);
       psr->advance(psr);
   }

    return left;
}

Node::Expr *ParserClass::num(Parser *psr) {
    return new NumberExpr(psr->current(psr).value);
}

Node::Expr *ParserClass::ident(Parser *psr) {
    return new IdentExpr(psr->current(psr).value);
}

Node::Expr *ParserClass::str(Parser *psr) {
    return new StringExpr(psr->current(psr).value);
}

Node::Expr *ParserClass::unary(Parser *psr) {
    auto op = psr->current(psr).value;
    auto right = parseExpr(psr, BindingPower::prefix);
    return new UnaryExpr(right, op);
}

Node::Expr *ParserClass::group(Parser *psr) {
    psr->expect(psr, TokenKind::LEFT_PAREN);
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::RIGHT_PAREN);

    return new GroupExpr(expr);
}

Node::Expr *ParserClass::binary(Parser *psr, Node::Expr *left, std::string op, 
                                BindingPower bp) {
    auto right = parseExpr(psr, bp);
    return new BinaryExpr(left, right, op);
}