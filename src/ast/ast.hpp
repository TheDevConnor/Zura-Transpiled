#pragma once

#include "../lexer/lexer.hpp"
#include <vector>

enum class AstNodeType {
  // Program
  PROGRAM,
  // Expressions
  BINARY,
  GROUPING,
  NUMBER_LITERAL,
  STRING_LITERAL,
  TRUE_LITERAL,
  FALSE_LITERAL,
  NIL_LITERAL,
  UNARY,

  // Types
  TYPE,
  TYPE_STRUCT,

  // Statements
  EXPRESSION,
  PRINT,
  VAR_DECLARATION,
  FUNCTION_DECLARATION,
  BLOCK,
  EXIT,
};

class AstNode {
public:
  AstNode(AstNodeType type, void *data) : type(type), data(data) {}

  AstNodeType type;
  void *data;

  struct Expr {
    AstNode *left;
    AstNode *right;
  };

  struct Stmt {
    AstNode *expression;
  };

  struct Program {
    std::vector<AstNode *> statements;

    Program(std::vector<AstNode *> statements) : statements(statements) {}

    // Proveide the iterator interface
    std::vector<AstNode *>::iterator begin() { return statements.begin(); }
    std::vector<AstNode *>::iterator end() { return statements.end(); }
  };

  Program *program;

  // ! Types
  struct Type {
    Lexer::Token type;

    Type(Lexer::Token type) : type(type) {}
  };

  struct TypeStruct : public Type {
    Lexer::Token name;
    std::vector<AstNode *> fields;

    TypeStruct(Lexer::Token type, std::vector<AstNode *> fields)
        : Type(type), fields(fields) {}
  };

  // ! Expressions
  struct Binary : public Expr {
    AstNode *left;
    TokenKind op;
    AstNode *right;

    Binary(AstNode *left, TokenKind op, AstNode *right)
        : op(op), left(left), right(right) {}
  };
  struct Unary : public Expr {
    TokenKind op;
    AstNode *right;

    Unary(TokenKind op, AstNode *right) : op(op), right(right) {}
  };
  struct Grouping : public Expr {
    AstNode *expression;

    Grouping(AstNode *expression) : expression(expression) {}
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
    AstNode *expression;

    Expression(AstNode *expression) : expression(expression) {}
  };
  struct FunctionDeclaration : public Stmt {
    Lexer::Token name;
    std::vector<Lexer::Token> parameters;
    std::vector<AstNode *> paramType;
    AstNode *type;
    AstNode *body;

    FunctionDeclaration(Lexer::Token name, std::vector<Lexer::Token> parameters,
                        std::vector<AstNode *> paramType, AstNode *type,
                        AstNode *body)
        : name(name), parameters(parameters), paramType(paramType), type(type),
          body(body) {}
  };
  struct Block : public Stmt {
    std::vector<AstNode *> statements;

    Block(std::vector<AstNode *> statements) : statements(statements) {}
  };
  struct VarDeclaration : public Stmt {
    Lexer::Token name;
    AstNode *type;
    AstNode *initializer;

    VarDeclaration(Lexer::Token name, AstNode *type, AstNode *initializer)
        : name(name), type(type), initializer(initializer) {}
  };
  struct Print : public Stmt {
    AstNode *expression;
    Lexer::Token ident;

    Print(AstNode *expression, Lexer::Token ident)
        : expression(expression), ident(ident) {}
  };
  struct Exit : public Stmt {
    AstNode *expression;

    Exit(AstNode *expression) : expression(expression) {}
  };

  static void printVarDeclaration(AstNode::VarDeclaration *varDeclaration,
                                  int indent);
  static void printAst(AstNode *node, int indent);
};
