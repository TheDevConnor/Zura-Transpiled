#pragma once

#include <map>
#include <memory>
#include <vector>

#include "../lexer/lexer.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

using namespace llvm;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;
static std::map<std::string, Value *> NamedValues;

void InitBuilder();

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
  TYPE_POINTER,
  TYPE_ARRAY,

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
  virtual AstNodeType getNodeType() const = 0;
  virtual Value *codegen() = 0;
};

class StmtAST {
public:
  virtual ~StmtAST() = default;
  virtual AstNodeType getNodeType() const = 0;
  virtual Value *codegen() = 0;
};

//! Expressions
class BinaryExprAST : public ExprAST {
public:
  BinaryExprAST(std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS,
                TokenKind Op)
      : LHS(std::move(LHS)), RHS(std::move(RHS)), Op(Op) {}
  AstNodeType getNodeType() const override { return AstNodeType::BINARY; }
  std::unique_ptr<ExprAST> LHS;
  std::unique_ptr<ExprAST> RHS;
  TokenKind Op;
  ~BinaryExprAST() override = default;
  Value *codegen() override;
};
class LiteralExprAST : public ExprAST {
public:
  LiteralExprAST(Lexer::Token value) : value(value) {}
  AstNodeType getNodeType() const override { return AstNodeType::NUMBER_LITERAL; }
  Lexer::Token value;
  ~LiteralExprAST() override = default;
  Value *codegen() override;
};
class NumberExprAST : public ExprAST {
public:
  NumberExprAST(double Val) : Val(Val) {}
  AstNodeType getNodeType() const override { return AstNodeType::NUMBER_LITERAL; }
  double Val;
  ~NumberExprAST() override = default;
  Value *codegen() override;
};
class UnaryExprAST : public ExprAST {
public:
  UnaryExprAST(std::unique_ptr<ExprAST> RHS, TokenKind Op)
      : RHS(std::move(RHS)), Op(Op) {}
  AstNodeType getNodeType() const override { return AstNodeType::UNARY; }
  std::unique_ptr<ExprAST> RHS;
  TokenKind Op;
  ~UnaryExprAST() override = default;
  Value *codegen() override;
};
class GroupingExprAST : public ExprAST {
public:
  GroupingExprAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() const override { return AstNodeType::GROUPING; }
  std::unique_ptr<ExprAST> Expr;
  ~GroupingExprAST() override = default;
  Value *codegen() override;
};
class CallExprAST : public ExprAST {
public:
  CallExprAST(std::unique_ptr<ExprAST> Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(std::move(Callee)), Args(std::move(Args)) {}
  AstNodeType getNodeType() const override { return AstNodeType::CALL; }
  std::unique_ptr<ExprAST> Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;
  ~CallExprAST() override = default;
  Value *codegen() override;
};
class IdentifierExprAST : public ExprAST {
public:
  IdentifierExprAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() const override { return AstNodeType::IDENTIFIER; }
  std::string Name;
  ~IdentifierExprAST() override = default;
  Value *codegen() override;
};

//! Types
class TypeAST : public ExprAST {
public:
  TypeAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() const override { return AstNodeType::TYPE; }
  std::string Name;
  ~TypeAST() override = default;
  Value *codegen() override;
};
class TypePointerAST : public ExprAST {
public:
  TypePointerAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() const override { return AstNodeType::TYPE_POINTER; }
  std::string Name;
  ~TypePointerAST() override = default;
  Value *codegen() override;
};
class TypeArrayAST : public ExprAST {
public:
  TypeArrayAST(std::string Name) : Name(Name) {}
  AstNodeType getNodeType() const override { return AstNodeType::TYPE_ARRAY; }
  std::string Name;
  ~TypeArrayAST() override = default;
  Value *codegen() override;
};

//! Array
class ArrayTypeAST : public ExprAST {
public:
  ArrayTypeAST(std::unique_ptr<TypeAST> Type) : Type(std::move(Type)) {}
  AstNodeType getNodeType() const override { return AstNodeType::ARRAY_TYPE; }
  std::unique_ptr<TypeAST> Type;
  ~ArrayTypeAST() override = default;
  Value *codegen() override;
};
class ArrayExprAST : public ExprAST {
public:
  ArrayExprAST(std::vector<std::unique_ptr<ExprAST>> Elements)
      : Elements(std::move(Elements)) {}
  AstNodeType getNodeType() const override { return AstNodeType::ARRAY; }
  std::vector<std::unique_ptr<ExprAST>> Elements;
  ~ArrayExprAST() override = default;
  Value *codegen() override;
};

//! Statements
class ExpressionStmtAST : public StmtAST {
public:
  ExpressionStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() const override { return AstNodeType::EXPRESSION; }
  std::unique_ptr<ExprAST> Expr;
  ~ExpressionStmtAST() override = default;
  Value *codegen() override;
};
class PrintStmtAST : public StmtAST {
public:
  PrintStmtAST(std::unique_ptr<ExprAST> Expr,
               std::vector<std::unique_ptr<ExprAST>> idents)
      : Expr(std::move(Expr)), Idents(std::move(idents)) {}
  AstNodeType getNodeType() const override { return AstNodeType::PRINT; }
  std::unique_ptr<ExprAST> Expr;
  std::vector<std::unique_ptr<ExprAST>> Idents;
  ~PrintStmtAST() override = default;
  Value *codegen() override;
};
class VarDeclStmtAST : public StmtAST {
public:
  VarDeclStmtAST(Lexer::Token Name, std::unique_ptr<TypeAST> Type,
                 std::unique_ptr<ExprAST> Expr)
      : Name(Name), Type(std::move(Type)), Expr(std::move(Expr)) {}
  AstNodeType getNodeType() const override {
    return AstNodeType::VAR_DECLARATION;
  }
  Lexer::Token Name;
  std::unique_ptr<TypeAST> Type;
  std::unique_ptr<ExprAST> Expr;
  ~VarDeclStmtAST() override = default;
  Value *codegen() override;
};
class FunctionDeclStmtAST : public StmtAST {
public:
  FunctionDeclStmtAST(std::string Name, std::vector<std::string> Params,
                      std::vector<TypeAST *> ParamTypes, TypeAST *ResultType,
                      std::unique_ptr<StmtAST> Body)
      : Name(Name), Params(std::move(Params)),
        ParamTypes(std::move(ParamTypes)), ResultType(ResultType),
        Body(std::move(Body)) {}
  AstNodeType getNodeType() const override {
    return AstNodeType::FUNCTION_DECLARATION;
  }
  std::string Name;
  std::vector<std::string> Params;
  std::vector<TypeAST *> ParamTypes;
  TypeAST *ResultType;
  std::unique_ptr<StmtAST> Body;
  ~FunctionDeclStmtAST() override = default;
  Value *codegen() override;
};
class BlockStmtAST : public StmtAST {
public:
  BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> Stmts)
      : Stmts(std::move(Stmts)) {}
  AstNodeType getNodeType() const override { return AstNodeType::BLOCK; }
  std::vector<std::unique_ptr<StmtAST>> Stmts;
  ~BlockStmtAST() override = default;
  Value *codegen() override;
};
class ExitStmtAST : public StmtAST {
public:
  ExitStmtAST(std::unique_ptr<ExprAST> value) : value(std::move(value)) {}
  std::unique_ptr<ExprAST> value;
  AstNodeType getNodeType() const override { return AstNodeType::EXIT; }
  ~ExitStmtAST() override = default;
  Value *codegen() override;
};
class ReturnStmtAST : public StmtAST {
public:
  ReturnStmtAST(std::unique_ptr<ExprAST> Expr) : Expr(std::move(Expr)) {}
  AstNodeType getNodeType() const override { return AstNodeType::RETURN; }
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
  virtual Value *codegen() = 0;
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

