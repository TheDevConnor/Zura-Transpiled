#pragma once

#include <unordered_map>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

namespace ParserClass {
    enum BindingPower {
        defaultValue = -1,
        comma = 0,
        assignment = 1,
        ternary = 2,
        logicalOr = 3,
        logicalAnd = 4,
        relational = 5,
        comparison = 6,
        additive = 7,
        multiplicative = 8,
        power = 9,
        prefix = 10,
        postfix = 11,
        call = 12,
        field = 13,
        err = 14
    };
    struct Parser;

    using nud_t = Node::Expr* (*)(Parser*);
    using led_t = Node::Expr* (*)(Parser*, Node::Expr*, std::string, BindingPower);
}

struct ParserClass::Parser {
    std::vector<Lexer::Token> tks;
    size_t pos = 0;

    Lexer::Token current(Parser *psr) {
        return psr->tks[psr->pos];
    }

    Lexer::Token advance(Parser *psr) {
        if (psr->pos >= psr->tks.size())
            return current(psr);
        return psr->tks[psr->pos++];
    }

    Lexer::Token peek(Parser *psr, int offset = 0) {
        return psr->tks[psr->pos + offset];
    }

    bool expect(Parser *psr, TokenKind tk) {
        bool result = (current(psr).kind == tk) ? advance(psr), true : false;
        if (!result)
            std::cerr << "Expected token " << tk << " but got " << current(psr).kind << std::endl;
        return result;
    }
};

namespace ParserClass {
    Node::Stmt parse(const char *source);

    // Tables for the Pratt parser and their lookup functions.
    extern const std::unordered_map<TokenKind, BindingPower> bp_table;
    extern const std::unordered_map<TokenKind, nud_t> nud_table;
    extern const std::unordered_map<TokenKind, led_t> led_table;

    BindingPower getBP(Parser *psr, TokenKind tk);
    Node::Expr *nudHandler(Parser *psr, TokenKind tk);
    Node::Expr *ledHandler(Parser *psr, Node::Expr *left);

    // Pratt parser functions.
    void storeToken(Parser *psr, Lexer *lex, Lexer::Token tk);
    Node::Expr *parseExpr(Parser *psr, BindingPower bp);

    // Expr Functions
    Node::Expr *num(Parser *psr);
    Node::Expr *ident(Parser *psr);
    Node::Expr *str(Parser *psr);
    Node::Expr *unary(Parser *psr);
    Node::Expr *group(Parser *psr);
    Node::Expr *binary(Parser *psr, Node::Expr *left, std::string op,
                       BindingPower bp);
}