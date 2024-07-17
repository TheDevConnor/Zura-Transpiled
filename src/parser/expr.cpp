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
        ErrorClass::error(psr->current(psr).line, psr->current(psr).column, 
                          "Could not parse primary expression!", "", "Parser Error", "main.zu", 
                          lexer, psr->tks, true, false, false, false); 
        return nullptr;
    }
}

Node::Expr *Parser::group(PStruct *psr) {
    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected L_Paran before a grouping expr!");
    auto *expr = parseExpr(psr, defaultValue);
    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected R_Paran after a grouping expr!");
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

Node::Expr *Parser::_prefix(PStruct *psr) {
    auto op = psr->advance(psr);
    auto *right = parseExpr(psr, defaultValue);

    return new PrefixExpr(
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

Node::Expr *Parser::parse_call(PStruct *psr, Node::Expr *left, BindingPower bp) {
    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_Paran to start a call expr!");
    std::vector<Node::Expr *> args;

    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        args.push_back(parseExpr(psr, defaultValue));
        if (psr->current(psr).kind == TokenKind::COMMA)
            psr->expect(psr, TokenKind::COMMA, "Expected a COMMA after an arguement!");
    }

    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_Paren to end a call expr!");

    return new CallExpr(
        left,
        args
    );
}

Node::Expr *Parser::_ternary(PStruct *psr, Node::Expr *left, BindingPower bp) {
    psr->expect(psr, TokenKind::QUESTION, "Expected a '?' to define the true value of an ternary expr!");
    auto *true_expr = parseExpr(psr, defaultValue);
    psr->expect(psr, TokenKind::COLON, "Expected a ':' to define the false value of a ternary Expr!");
    auto *false_expr = parseExpr(psr, defaultValue);

    return new TernaryExpr(
        left,
        true_expr,
        false_expr
    );
}

Node::Expr *Parser::_member(PStruct *psr, Node::Expr *left, BindingPower bp) {
   auto op = psr->advance(psr);
   auto *right = parseExpr(psr, defaultValue);

   return new MemberExpr(
       left,
       right
   );
}