#pragma once

#include "../lexer/lexer.hpp"

enum class AstNodeType {
    // Expressions
    BINARY,
    GROUPING,
    LITERAL,
    UNARY,

    // Statements
    EXPRESSION,
    PRINT,
    VAR_DECLARATION,
};

class AstNode {
public:
    AstNodeType type;

    struct Expr {
        AstNode* left;
        AstNode* right;
    };

    struct Stmt { AstNode* expression; };

    struct Binary : public Expr { TokenKind op; AstNode* left; AstNode* right;};
    struct Unary : public Expr { TokenKind op; AstNode* right; };
    struct Grouping : public Expr { AstNode* expression; };
    struct Literal : public Expr { Lexer::Token literal; };

    struct VarDeclaration : public Stmt { Lexer::Token name; AstNode* initializer; };
    struct Expression : public Stmt { AstNode* expression; };
    struct Print : public Stmt { AstNode* expression; };
};
