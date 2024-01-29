#include "visit.hpp"
#include "../ast/ast.hpp"

void AstVisitor::visit(const AstNode &node) {
  switch (node.type) {
  case AstNodeType::PROGRAM:
    visitProgram(node);
    break;
  default:
    break;
  }
}

void AstVisitor::visitProgram(const AstNode &node) {
  for (const auto &stmt : node.Stmts) {
    visitStmt(*stmt);
  }
}

void AstVisitor::visitStmt(const StmtAST &stmt) {
  switch (stmt.getNodeType()) {
  case AstNodeType::FUNCTION_DECLARATION:
    visitFuncDec(static_cast<const FunctionDeclStmtAST &>(stmt));
    break;
  default:
    break;
  }
}

void AstVisitor::visitFuncDec(const FunctionDeclStmtAST &func) {
  size_t pos = func.Name.find('(');

  std::string functionName =
      (pos != std::string::npos) ? func.Name.substr(0, pos) : func.Name;

  if (functionName == "main") {
    foundMain = true;
    return;
  }
  foundMain = false;
}
