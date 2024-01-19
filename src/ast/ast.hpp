#pragma once

#include <vector>
#include <memory>

#include "llvm/IR/Value.h"
#include "../lexer/lexer.hpp"

using namespace llvm;

enum class AstNodeType {
  // Program
  PROGRAM,
  // Expressions
  BINARY,
  GROUPING,
  CALL,
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

  // Array
  ARRAY_TYPE,
  ARRAY,

  // Statements
  EXPRESSION,
  PRINT,
  VAR_DECLARATION,
  FUNCTION_DECLARATION,
  BLOCK,
  EXIT,
  RETURN,
};

class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual AstNodeType getNodeType() = 0;
  virtual Value *codegen() = 0;
};

class StmtAST {
public:
  virtual ~StmtAST() = default;
  virtual AstNodeType getNodeType() = 0;
  virtual Value *codegen() = 0;
};

//! Expressions
class BinaryExprAST : public ExprAST {
public:
  BinaryExprAST(std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS,
                TokenKind Op)
      : LHS(std::move(LHS)), RHS(std::move(RHS)), Op(Op) {}
  AstNodeType getNodeType() override { return AstNodeType::BINARY; }
  std::unique_ptr<ExprAST> LHS;
  std::unique_ptr<ExprAST> RHS;
  TokenKind Op;
  ~BinaryExprAST() override = default;
  Value *codegen() override;
};
class LiteralExprAST : public ExprAST {
public:
  LiteralExprAST(Lexer::Token value) : value(value) {}
  AstNodeType getNodeType() override { return AstNodeType::NUMBER_LITERAL; }
  Lexer::Token value;
  ~LiteralExprAST() override = default;
  Value *codegen() override;
};
class UnaryExprAST : public ExprAST {
public:
  UnaryExprAST(std::unique_ptr<ExprAST> RHS, TokenKind Op)
      : RHS(std::move(RHS)), Op(Op) {}
  AstNodeType getNodeType() override { return AstNodeType::UNARY; }
  std::unique_ptr<ExprAST> RHS;
  TokenKind Op;
  ~UnaryExprAST() override = default;
  Value *codegen() override;
};
class GroupingExprAST : public ExprAST {
public:
  GroupingExprAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() override { return AstNodeType::GROUPING; }
  std::unique_ptr<ExprAST> Expr;
  ~GroupingExprAST() override = default;
  Value *codegen() override;
};
class CallExprAST : public ExprAST {
public:
  CallExprAST(std::unique_ptr<ExprAST> Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(std::move(Callee)), Args(std::move(Args)) {}
  AstNodeType getNodeType() override { return AstNodeType::CALL; }
  std::unique_ptr<ExprAST> Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;
  ~CallExprAST() override = default;
  Value *codegen() override;
};
class IdentifierExprAST : public ExprAST {
public:
  IdentifierExprAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() override { return AstNodeType::IDENTIFIER; }
  std::string Name;
  ~IdentifierExprAST() override = default;
  Value *codegen() override;
};

//! Types
class TypeAST : public ExprAST {
public:
  TypeAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() override { return AstNodeType::TYPE; }
  std::string Name;
  ~TypeAST() override = default;
  Value *codegen() override;
};
class StructTypeAST : public ExprAST {
public:
  StructTypeAST(std::string Name, std::vector<std::unique_ptr<TypeAST>> Fields)
      : Name(Name), Fields(std::move(Fields)) {}
  AstNodeType getNodeType() override { return AstNodeType::TYPE_STRUCT; }
  std::string Name;
  std::vector<std::unique_ptr<TypeAST>> Fields;
  ~StructTypeAST() override = default;
  Value *codegen() override;
};

//! Array
class ArrayTypeAST : public ExprAST {
public:
  ArrayTypeAST(std::unique_ptr<TypeAST> Type)
      : Type(std::move(Type)) {}
  AstNodeType getNodeType() override { return AstNodeType::ARRAY_TYPE; }
  std::unique_ptr<TypeAST> Type;
  ~ArrayTypeAST() override = default;
  Value *codegen() override;
};
class ArrayExprAST : public ExprAST {
public:
  ArrayExprAST(std::vector<std::unique_ptr<ExprAST>> Elements)
      : Elements(std::move(Elements)) {}
  AstNodeType getNodeType() override { return AstNodeType::ARRAY; }
  std::vector<std::unique_ptr<ExprAST>> Elements;
  ~ArrayExprAST() override = default;
  Value *codegen() override;
};

//! Statements
class ExpressionStmtAST : public StmtAST {
public:
  ExpressionStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() override { return AstNodeType::EXPRESSION; }
  std::unique_ptr<ExprAST> Expr;
  ~ExpressionStmtAST() override = default;
  Value *codegen() override;
};
class PrintStmtAST : public StmtAST {
public:
  PrintStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() override { return AstNodeType::PRINT; }
  std::unique_ptr<ExprAST> Expr;
  ~PrintStmtAST() override = default;
  Value *codegen() override;
};
class VarDeclStmtAST : public StmtAST {
public:
  VarDeclStmtAST(std::string Name, std::unique_ptr<ExprAST> Expr)
      : Name(Name), Expr(std::move(Expr)) {}
  AstNodeType getNodeType() override { return AstNodeType::VAR_DECLARATION; }
  std::string Name;
  std::unique_ptr<ExprAST> Expr;
  ~VarDeclStmtAST() override = default;
  Value *codegen() override;
};
class FunctionDeclStmtAST : public StmtAST {
public:
  FunctionDeclStmtAST(std::string Name, std::vector<std::string> Params,
                      std::unique_ptr<ExprAST> Body)
      : Name(Name), Params(std::move(Params)), Body(std::move(Body)) {}
  AstNodeType getNodeType() override { return AstNodeType::FUNCTION_DECLARATION; }
  std::string Name;
  std::vector<std::string> Params;
  std::unique_ptr<ExprAST> Body;
  ~FunctionDeclStmtAST() override = default;
  Value *codegen() override;
};
class BlockStmtAST : public StmtAST {
public:
  BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}
  AstNodeType getNodeType() override { return AstNodeType::BLOCK; }
  std::vector<std::unique_ptr<StmtAST>> Stmts;
  ~BlockStmtAST() override = default;
  Value *codegen() override;
};
class ExitStmtAST : public StmtAST {
public:
  ExitStmtAST() {}
  AstNodeType getNodeType() override { return AstNodeType::EXIT; }
  ~ExitStmtAST() override = default;
  Value *codegen() override;
};
class ReturnStmtAST : public StmtAST {
public:
  ReturnStmtAST(std::unique_ptr<ExprAST> Expr)
      : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() override { return AstNodeType::RETURN; }
  std::unique_ptr<ExprAST> Expr;
  ~ReturnStmtAST() override = default;
  Value *codegen() override;
};

//! Program
class ProgramAST {
public:
  ProgramAST(std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}
  std::vector<std::unique_ptr<StmtAST>> Stmts;
  ~ProgramAST() = default;
  void codegen();
};

class AstNode {
public:
  AstNode(AstNodeType type, std::vector<std::unique_ptr<StmtAST>> Stmts)
      : type(type), Stmts(std::move(Stmts)) {}
  AstNode(AstNodeType type, std::vector<std::unique_ptr<ExprAST>> Exprs)
      : type(type), Exprs(std::move(Exprs)) {}
  AstNode(AstNodeType type, std::vector<std::unique_ptr<TypeAST>> Types)
      : type(type), Types(std::move(Types)) {}
  AstNode(AstNodeType type, std::vector<std::unique_ptr<ExprAST>> Exprs,
          std::vector<std::unique_ptr<StmtAST>> Stmts)
      : type(type), Exprs(std::move(Exprs)), Stmts(std::move(Stmts)) {}
  AstNodeType type;
  std::vector<std::unique_ptr<ExprAST>> Exprs;
  std::vector<std::unique_ptr<StmtAST>> Stmts;
  std::vector<std::unique_ptr<TypeAST>> Types;
};

