#pragma once

#include <iostream>
#include <string>

enum NodeKind {
    // Expressions
    ND_NUMBER,
    ND_IDENT,
    ND_STRING,
    ND_BINARY,
    ND_UNARY,
    ND_PREFIX,
    ND_GROUP,
    ND_CALL,
    ND_ASSIGN,
    ND_TERNARY,
    ND_MEMBER,

    // Statements
    ND_EXPR_STMT,
    ND_VAR_STMT,
    ND_CONST_STMT,
    ND_BLOCK_STMT,
    ND_FN_STMT,
    ND_PROGRAM,
    ND_RETURN_STMT,
    ND_IF_STMT,
    ND_STRUCT_STMT,
    ND_WHILE_STMT,
    ND_FOR_STMT,
    ND_PRINT_STMT,

    // Types
    ND_SYMBOL_TYPE,
    ND_ARRAY_TYPE,
    ND_POINTER_TYPE,
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
        virtual void debug(int ident = 0) const = 0;
        virtual ~Stmt() = default;
    };

    struct Type {
        NodeKind kind;
        virtual void debug(int ident = 0) const = 0;
        virtual ~Type() = default;
    };

    static void printIndent(int ident) {
        for (int i = 0; i < ident; i++) {
            std::cout << "    ";
        }
    }
};
 