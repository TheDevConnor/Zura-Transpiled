#include "../ast/ast.hpp"
#include "type.hpp"

#include <algorithm>
#include <functional>
#include <vector>

using namespace TypeChecker;

void TypeChecker::lookup(callables_table &ctable, symbol_table &table,
                         Node::Stmt *stmt) {
  auto it = std::find_if(stmts.begin(), stmts.end(),
                         [stmt](auto &s) { return s.first == stmt->kind; });

  if (it != stmts.end()) {
    it->second(ctable, table, stmt);
  }
}

void TypeChecker::lookup(callables_table &ctable, symbol_table &table,
                         Node::Expr *expr) {
  auto it = std::find_if(exprs.begin(), exprs.end(),
                         [expr](auto &e) { return e.first == expr->kind; });

  if (it != exprs.end()) {
    it->second(ctable, table, expr);
  }
}

std::vector<std::pair<NodeKind, TypeChecker::StmtNodeHandler>>
    TypeChecker::stmts = {
        {NodeKind::ND_PROGRAM, visitProgram},
        {NodeKind::ND_FN_STMT, visitFn},
        {NodeKind::ND_CONST_STMT, visitConst},
        {NodeKind::ND_BLOCK_STMT, visitBlock},
        {NodeKind::ND_VAR_STMT, visitVar},
        {NodeKind::ND_RETURN_STMT, visitReturn},
        {NodeKind::ND_IMPORT_STMT, visitImport},
        {NodeKind::ND_EXPR_STMT,
         [](callables_table &ctables, symbol_table &table, Node::Stmt *stmt) {
           auto expr_stmt = static_cast<ExprStmt *>(stmt);
           visitExpr(ctables, table, expr_stmt->expr);
         }},
};

std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>
    TypeChecker::exprs = {
        {NodeKind::ND_NUMBER, visitNumber}, {NodeKind::ND_STRING, visitString},
        {NodeKind::ND_IDENT, visitIdent},   {NodeKind::ND_BINARY, visitBinary},
        {NodeKind::ND_CALL, visitCall}, {NodeKind::ND_UNARY, visitUnary},
        {NodeKind::ND_GROUP, visitGrouping},
};