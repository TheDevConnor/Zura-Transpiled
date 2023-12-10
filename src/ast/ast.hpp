#pragma once

#include "../lexer/lexer.hpp"

enum class AstNodeType {
    // Expressions
    BINARY,
    GROUPING,
    NUMBER_LITERAL,
    STRING_LITERAL,
    TRUE_LITERAL,
    FALSE_LITERAL,
    NIL_LITERAL,
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
        AstNode* left; 
        TokenKind op; 
        AstNode* right; 

        Binary(AstNode* left, TokenKind op, AstNode* right) : op(op), left(left), right(right) {}
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

    struct NumberLiteral : public Expr { 
        double value; 

        NumberLiteral(double value) : value(value) {}
    };
    struct StringLiteral : public Expr { 
        std::string value; 

        StringLiteral(std::string value) : value(value) {}
    };
    struct TrueLiteral : public Expr { 
        TrueLiteral() {}
    };
    struct FalseLiteral : public Expr { 
        FalseLiteral() {}
    };
    struct NilLiteral : public Expr { 
        NilLiteral() {}
    };

    // ! Statements
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
