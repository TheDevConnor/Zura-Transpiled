#pragma once

#include <unordered_map>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

namespace Parser {
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
        _primary = 14,
        err = 15
    };
    struct PStruct;
}

struct Parser::PStruct {
    std::vector<Lexer::Token> tks;
    int pos = 0;

    Lexer::Token current(PStruct *psr) {
        return psr->tks[psr->pos];
    }

    Lexer::Token advance(PStruct *psr) {
        if (psr->pos + 1 < psr->tks.size()) {
            psr->pos++;
            return psr->current(psr);
        }
        return psr->current(psr);
    }

    Lexer::Token peek(PStruct *psr, int offset = 0) {
        return psr->tks[psr->pos + offset];
    }

    bool hasTokens(PStruct *psr) {
        return psr->pos < psr->tks.size();
    }

    bool expect(PStruct *psr, TokenKind tk) {
        bool res = current(psr).kind == tk ? true : false;
        if (res == false) {
            std::cerr << "Expected token " << tk << " but got " << current(psr).kind << std::endl;
            return false;
        }
        advance(psr);
        return res;
    }
};

namespace Parser {
    template <typename T, typename U>
    T lookup(const std::vector<std::pair<U, T>>& lu, U key);

    Node::Stmt *parse(const char *source);

    // Maps for the Pratt Parser
    using StmtHandler = std::function<Node::Stmt *(PStruct *)>;
    using NudHandler = std::function<Node::Expr *(PStruct *)>;
    using LedHandler = std::function<Node::Expr *(PStruct *, 
                                                  Node::Expr *, 
                                                  BindingPower)>;

    static std::vector<std::pair<TokenKind, StmtHandler>> stmt_lu;
    static std::vector<std::pair<TokenKind, NudHandler>> nud_lu;
    static std::vector<std::pair<TokenKind, LedHandler>> led_lu;
    static std::vector<std::pair<TokenKind, BindingPower>> bp_lu;
    void createMaps();

    Node::Expr *led(PStruct *psr, Node::Expr *left, BindingPower bp);
    BindingPower getBP(TokenKind tk);
    Node::Stmt *stmt(PStruct *psr);
    Node::Expr *nud(PStruct *psr);
    

    // Pratt parser functions.
    PStruct *setupParser(PStruct *psr, Lexer *lex, Lexer::Token tk); 
    Node::Expr *parseExpr(PStruct *psr, BindingPower bp);

    // Expr Functions
    Node::Expr *primary(PStruct * psr);
    Node::Expr *unary(PStruct *psr);
    Node::Expr *group(PStruct *psr);
    Node::Expr *binary(PStruct *psr, Node::Expr *left, BindingPower bp);

    // Stmt Functions
    Node::Stmt *parseStmt(PStruct *psr);
    Node::Stmt *exprStmt(PStruct *psr);
    Node::Stmt *varStmt(PStruct *psr);
}
