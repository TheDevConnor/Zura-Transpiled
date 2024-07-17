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
        {NodeKind::ND_EXPR_STMT,
         [](callables_table &ctables, symbol_table &table, Node::Stmt *stmt) {
           auto expr_stmt = static_cast<ExprStmt *>(stmt);
           visitExpr(ctables, table, expr_stmt->expr);
         }},
};

std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>
    TypeChecker::exprs = {
        {NodeKind::ND_NUMBER,
         [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
           auto number = static_cast<NumberExpr *>(expr);
           return_type = new SymbolType("int");
         }},
        {NodeKind::ND_STRING,
         [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
           auto string = static_cast<StringExpr *>(expr);
           return_type = new SymbolType("string");
         }},
        {NodeKind::ND_IDENT,
         [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
           auto identifier = static_cast<IdentExpr *>(expr);
           return_type = table_lookup(table, identifier->name);
         }},
        {NodeKind::ND_BINARY,
         [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
           auto binary = static_cast<BinaryExpr *>(expr);
           visitExpr(ctables, table, binary->lhs);
           visitExpr(ctables, table, binary->rhs);
         }},
        {NodeKind::ND_CALL,
         [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
           auto call = static_cast<CallExpr *>(expr);

           visitExpr(ctables, table, call->callee);

           auto params = table_lookup(
               ctables, static_cast<IdentExpr *>(call->callee)->name);

           if (params.size() != call->args.size()) {
             std::string msg =
                 "Function '" + static_cast<IdentExpr *>(call->callee)->name +
                 "' expects " + std::to_string(params.size()) +
                 " arguments but got " + std::to_string(call->args.size());
             handlerError(msg);
           }

           for (int i = 0; i < params.size(); i++) {
             visitExpr(ctables, table, call->args[i]);
             if (type_to_string(params[i].second) !=
                 type_to_string(return_type)) {
               std::string msg =
                   "Function '" + static_cast<IdentExpr *>(call->callee)->name +
                   "' expects argument " + std::to_string(i) +
                   " to be of type " + type_to_string(params[i].second) +
                   " but got " + type_to_string(return_type);
               handlerError(msg);
             }
           }

           return_type = table_lookup(
               table, static_cast<IdentExpr *>(call->callee)->name);
         }},
};