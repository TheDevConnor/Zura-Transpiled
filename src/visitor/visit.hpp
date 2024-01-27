#pragma once

#include "../ast/ast.hpp"

inline bool foundMain;

class AstVisitor {
public:
  void visit(const AstNode &node);

private:
  void visitProgram(const AstNode &node);
  void visitStmt(const StmtAST &stmt);
  void visitFuncDec(const FunctionDeclStmtAST &func);
};
