#pragma once

#include <unordered_map>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

namespace ParserNamespace {
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
    struct Parser;
}

struct ParserNamespace::Parser {
    std::vector<Lexer::Token> tks;
    int pos = 0;

    Lexer::Token current(Parser *psr) {
        return psr->tks[psr->pos];
    }

    Lexer::Token advance(Parser *psr) {
        if (psr->pos + 1 < psr->tks.size()) {
            psr->pos++;
            return psr->current(psr);
        }
        return psr->current(psr);
    }

    Lexer::Token peek(Parser *psr, int offset = 0) {
        return psr->tks[psr->pos + offset];
    }

    bool hasTokens(Parser *psr) {
        return psr->pos < psr->tks.size();
    }

    bool expect(Parser *psr, TokenKind tk) {
        if (current(psr).kind == tk) {
            advance(psr);
            return true;
        }
        std::cerr << "Expected token " << tk << " but got " << current(psr).kind << std::endl;
        return false;
    }
};

namespace ParserNamespace {
    Node::Stmt *parse(const char *source);

    // Maps for the Pratt Parser
    using StmtHandler = std::function<Node::Stmt(Parser*)>;
    using NudHandler = std::function<Node::Expr*(Parser*)>;
    using LedHandler = std::function<Node::Expr*(Parser*, Node::Expr*, BindingPower)>;

    using StmtLookup = std::unordered_map<TokenKind, StmtHandler>;
    using NudLookup = std::unordered_map<TokenKind, NudHandler>;
    using LedLookup = std::unordered_map<TokenKind, LedHandler>;
    using BpLookup = std::unordered_map<TokenKind, BindingPower>;

    extern BpLookup bp_lu;
    extern NudLookup nud_lu;
    extern LedLookup led_lu;
    extern StmtLookup stmt_lu;

    void led(TokenKind kind, BindingPower bp, LedHandler led_fn);
    void nud(TokenKind kind, NudHandler nud_fn);
    void stmt(TokenKind kind, StmtHandler stmt_fn);
    void createTokenLookup();

    // Pratt parser functions.
    std::vector<Lexer::Token> storeToken(Parser *psr, Lexer *lex, Lexer::Token tk);
    Parser *createParser(std::vector<Lexer::Token> tks);
    Node::Expr *parseExpr(Parser *psr, BindingPower bp);

    // Expr Functions
    Node::Expr *primary(Parser * psr);
    Node::Expr *unary(Parser *psr);
    Node::Expr *group(Parser *psr);
    Node::Expr *binary(Parser *psr, Node::Expr *left, BindingPower bp);

    // Stmt Functions
    Node::Stmt *parseStmt(Parser *psr);
    Node::Stmt *exprStmt(Parser *psr);
    Node::Stmt *varStmt(Parser *psr);
}
