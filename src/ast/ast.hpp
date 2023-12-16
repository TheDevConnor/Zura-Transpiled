#pragma once

#include "../lexer/lexer.hpp"
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

enum class AstNodeType {
  // Program
  PROGRAM,
  // Expressions 
  BINARY,
  GROUPING,
  IDENTIFIER,
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
    std::unique_ptr<AstNode> left;
    std::unique_ptr<AstNode> right;
  };

  struct Stmt {
    std::unique_ptr<AstNode> expression;
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
    std::unique_ptr<AstNode> left;
    TokenKind op;
    std::unique_ptr<AstNode> right;

    Binary(std::unique_ptr<AstNode> left, TokenKind op,
           std::unique_ptr<AstNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
  };
  struct Unary : public Expr {
    TokenKind op;
    std::unique_ptr<AstNode> right;

    Unary(TokenKind op, std::unique_ptr<AstNode> right)
        : op(op), right(std::move(right)) {}
  };
  struct Identifier : public Expr {
    Lexer::Token name;

    Identifier(Lexer::Token name) : name(name) {}
  };
  struct Grouping : public Expr {
    std::unique_ptr<AstNode> expression;

    Grouping(std::unique_ptr<AstNode> expression)
        : expression(std::move(expression)) {}
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
    std::unique_ptr<AstNode> expression;

    Expression(std::unique_ptr<AstNode> expression)
        : expression(std::move(expression)) {}
  };
  struct FunctionDeclaration : public Stmt {
    Lexer::Token name;
    std::vector<Lexer::Token> parameters;
    std::vector<AstNode *> paramType;
    std::unique_ptr<AstNode> type;
    std::unique_ptr<AstNode> body;

    FunctionDeclaration(Lexer::Token name, std::vector<Lexer::Token> parameters,
                        std::vector<AstNode *> paramType,
                        std::unique_ptr<AstNode> type,
                        std::unique_ptr<AstNode> body)
        : name(name), parameters(parameters), paramType(paramType),
          type(std::move(type)), body(std::move(body)) {}
  };
  struct Block : public Stmt {
    std::vector<AstNode *> statements;

    Block(std::vector<AstNode *> statements) : statements(statements) {}
  };
  struct VarDeclaration : public Stmt {
    Lexer::Token name;
    std::unique_ptr<AstNode> type;
    std::unique_ptr<AstNode> initializer;

    VarDeclaration(Lexer::Token name, std::unique_ptr<AstNode> type,
                   std::unique_ptr<AstNode> initializer)
        : name(name), type(std::move(type)),
          initializer(std::move(initializer)) {}
  };
  struct Print : public Stmt {
    std::unique_ptr<AstNode> expression;
    Lexer::Token ident;

    Print(std::unique_ptr<AstNode> expression, Lexer::Token ident)
        : expression(std::move(expression)), ident(ident) {}
  };
  struct Exit : public Stmt {
    std::unique_ptr<AstNode> expression;

    Exit(std::unique_ptr<AstNode> expression)
        : expression(std::move(expression)) {}
  };

  static void printVarDeclaration(AstNode::VarDeclaration *varDeclaration,
                                  int indent);
  // static void printAst(std::unique_ptr<AstNode> node, int indent);
};
