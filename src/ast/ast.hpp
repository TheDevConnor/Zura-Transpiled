#pragma once

#include <string>

enum NodeKind {
    // Expressions
    ND_NUMBER,
    ND_IDENT,
    ND_STRING,
    ND_BINARY,
    ND_UNARY,
    ND_GROUP,

    // Statements
    ND_EXPR_STMT,
    ND_VAR_STMT,
    ND_PROGRAM,
};

class Node {
public:
    struct Expr {
        NodeKind kind;
        virtual void debug() const = 0; 
    };

    struct Stmt {
        NodeKind kind;
        virtual void debug() const = 0;
    };
};
 