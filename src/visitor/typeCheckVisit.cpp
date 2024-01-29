#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
#include "../type/type.hpp"

#include "visit.hpp"
#include <iostream>
#include <memory>

void AstVisitor::typeCheckVisit(const AstNode &node) {
  switch (node.type) {
  case AstNodeType::PROGRAM:
    visitTypeCheckProgram(node);
    break;
  default:
    break;
  }
}

void AstVisitor::visitTypeCheckProgram(const AstNode &node) {
  for (const auto &stmt : node.Stmts)
    visitTypeCheckStmt(*stmt);
}

void AstVisitor::visitTypeCheckBlock(const BlockStmtAST &block) {
  for (const auto &stmt : block.Stmts)
    visitTypeCheckStmt(*stmt);
}

void AstVisitor::visitTypeCheckStmt(const StmtAST &stmt) {
  switch (stmt.getNodeType()) {
  case AstNodeType::FUNCTION_DECLARATION:
    visitTypeCheckFuncDec(static_cast<const FunctionDeclStmtAST &>(stmt));
    break;
  case AstNodeType::BLOCK:
    visitTypeCheckBlock(static_cast<const BlockStmtAST &>(stmt));
    break;
  case AstNodeType::EXIT:
    visitTypeCheckExit(static_cast<const ExitStmtAST &>(stmt));
    break;
  default:
    break;
  }
}

void AstVisitor::visitTypeCheckExpr(const ExprAST &expr) {
  TypeClass typeClass;
  switch (expr.getNodeType()) {
  case AstNodeType::NUMBER_LITERAL: {
    const auto &num = static_cast<const NumberExprAST &>(expr);
    typeClass.determineType(num.Val);
    break;
  }
  default:
    break;
  }
}

void AstVisitor::visitTypeCheckFuncDec(const FunctionDeclStmtAST &func) {
  type = std::make_unique<TypeAST>(func.ResultType->Name);
  visitTypeCheckStmt(*func.Body);

  std::string functionName = func.Name.substr(0, func.Name.find('('));

  std::cout << "function name: " << functionName << "\n";
  std::cout << "functionType: " << type->Name << "\n";
  std::cout << "returnType: " << returnType->Name << "\n";

  ErrorClass::errorType(type.get(), returnType.get(), functionName);
}

void AstVisitor::visitTypeCheckExit(const ExitStmtAST &exit) {
  visitTypeCheckExpr(*exit.value);
}
