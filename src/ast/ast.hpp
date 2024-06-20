#pragma once
#ifdef csk

#include <iostream>
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
        virtual void debug(int ident = 0) const = 0;
        virtual ~Expr() = default; 
    };

    struct Stmt {
        NodeKind kind;
        virtual void debug() const = 0;
        virtual ~Stmt() = default;
    };

    static void printIdent(int ident) {
        for (int i = 0; i < ident; i++) {
            std::cout << "    ";
        }
    }
};
#endif