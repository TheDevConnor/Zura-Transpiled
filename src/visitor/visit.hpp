#pragma once

#include "../ast/ast.hpp"

inline bool foundMain;

class AstVisitor {
public:
  void visit(const AstNode &node);
  void typeCheckVisit(const AstNode &node);

private:
  // Found main function
  void visitProgram(const AstNode &node);
  void visitStmt(const StmtAST &stmt);
  void visitFuncDec(const FunctionDeclStmtAST &func);

  // TypeCheck Visitor
  void visitTypeCheckProgram(const AstNode &node);
  void visitTypeCheckStmt(const StmtAST &stmt);
  void visitTypeCheckExpr(const ExprAST &expr);
  void visitTypeCheckBlock(const BlockStmtAST &block);
  void visitTypeCheckExit(const ExitStmtAST &exit);
  void visitTypeCheckFuncDec(const FunctionDeclStmtAST &func);
};
