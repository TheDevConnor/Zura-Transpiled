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
    AstNode(AstNodeType type, void* data) : type(type), data(data) {}

    AstNodeType type;
    void* data;

    struct Expr {
        AstNode* left;
        AstNode* right;
    };

    struct Stmt { AstNode* expression; };

    // ! Expressions
    struct Binary : public Expr { 
        TokenKind op; 
        AstNode* left; 
        AstNode* right; 

        Binary(TokenKind op, AstNode* left, AstNode* right) : op(op), left(left), right(right) {}
    };
    struct Unary : public Expr { 
        TokenKind op; 
        AstNode* right; 

        Unary(TokenKind op, AstNode* right) : op(op), right(right) {}
    };
    struct Grouping : public Expr { 
        AstNode* expression; 

        Grouping(AstNode* expression) : expression(expression) {}
    };
    struct Literal : public Expr { 
        Lexer::Token literal; 
        Literal(Lexer::Token literal) : literal(literal) {}
    };

    // ! Statements
    struct VarDeclaration : public Stmt { 
        Lexer::Token name; 
        AstNode* initializer; 

        VarDeclaration(Lexer::Token name, AstNode* initializer) : name(name), initializer(initializer) {}
    };
    struct Expression : public Stmt { 
        AstNode* expression;

        Expression(AstNode* expression) : expression(expression) {}
    };
    struct Print : public Stmt { 
        AstNode* expression; 

        Print(AstNode* expression) : expression(expression) {}
    };

    static void printAst(AstNode* node, int indent);
};
