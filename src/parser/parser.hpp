#pragma once

#include <sys/types.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

class ParserClass;

struct Parser {
    std::vector<Lexer::Token> tks;
    int pos = 0;

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
        if (current(psr).kind == tk) {
            advance(psr);
            return true;
        }
        std::cout << "Expected token " << tk << " but got "
                  << current(psr).kind << std::endl;
        return false;
    }
};

enum BindingPower {
    defaultValue = 0,
    comma = 1,
    assignment = 2,
    ternary = 3,
    logicalOr = 4,
    logicalAnd = 5,
    relational = 6,
    comparison = 7,
    additive = 8,
    multiplicative = 9,
    power = 10,
    prefix = 11,
    postfix = 12,
    call = 13,
    field = 14,
    err = 15
};

using nud_t = Node::Expr *(ParserClass::*)(Parser*);
using led_t = Node::Expr *(ParserClass::*)(Parser*, 
                                           Node::Expr*,
                                           std::string,
                                           BindingPower);

class ParserClass {
public:
    static Node::Stmt parse(const char *source);
private:
    Parser psr;

    // Tables for the Pratt parser and there lookup functions.
    static const std::unordered_map<TokenKind, BindingPower> bp_table;
    static const std::unordered_map<TokenKind, nud_t> nud_table;
    static const std::unordered_map<TokenKind, led_t> led_table;

    BindingPower getBP(Parser *psr, TokenKind tk);
    Node::Expr* nudHandler(Parser *psr, TokenKind tk);
    Node::Expr* ledHandler(Parser *psr, Node::Expr* left);

    // Pratt parser functions.
    static void storeToken(Parser *psr, Lexer *lex, Lexer::Token tk);
    Node::Expr* parseExpr(Parser *psr, BindingPower bp);

    // Expr Functions
    Node::Expr *num(Parser *psr);
    Node::Expr *ident(Parser *psr);
    Node::Expr *str(Parser *psr);
    Node::Expr *unary(Parser *psr);
    Node::Expr *group(Parser *psr);
    Node::Expr *binary(Parser *psr, Node::Expr* left, std::string op, 
                       BindingPower bp);
};
