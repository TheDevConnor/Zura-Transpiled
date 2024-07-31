#include "../ast/ast.hpp"
#include "type.hpp"

#include <algorithm>
#include <functional>
#include <vector>

using namespace TypeChecker;

Node::Stmt *TypeChecker::StmtAstLookup(Node::Stmt *node,
                                       global_symbol_table gTable,
                                       local_symbol_table lTable, 
                                       function_table fn_table) {
  for (auto &stmtHandler : stmts) {
    if (node->kind == stmtHandler.first) {
      stmtHandler.second(gTable, lTable, fn_table, node);
    }
  }
  return node;
}

Node::Expr *TypeChecker::ExprAstLookup(Node::Expr *node,
                                       global_symbol_table gTable,
                                       local_symbol_table lTable) {
  for (auto &exprHandler : exprs) {
    if (node->kind == exprHandler.first) {
      exprHandler.second(gTable, lTable, node);
    }
  }
  return node;
}

std::vector<std::pair<NodeKind, TypeChecker::StmtNodeHandler>>
    TypeChecker::stmts = {
        {NodeKind::ND_PROGRAM, visitProgram},
        {NodeKind::ND_CONST_STMT, visitConst},
        {NodeKind::ND_FN_STMT, visitFn},
        {NodeKind::ND_BLOCK_STMT, visitBlock},
        {NodeKind::ND_RETURN_STMT, visitReturn},
        {NodeKind::ND_EXPR_STMT,
         [](global_symbol_table &gTable, local_symbol_table &lTable,
            function_table &fn_table, Node::Stmt *stmt) {
           auto expr_stmt = static_cast<ExprStmt *>(stmt);
           visitExpr(gTable, lTable, expr_stmt->expr);
         }},
};

std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>
    TypeChecker::exprs = {
        {NodeKind::ND_NUMBER, visitNumber},
        {NodeKind::ND_IDENT, visitIdent},
        {NodeKind::ND_STRING, visitString},
        {NodeKind::ND_BINARY, visitBinary},
};