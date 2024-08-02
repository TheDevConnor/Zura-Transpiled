#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

void TypeChecker::visitStmt(Maps *map, Node::Stmt *stmt) {
  StmtAstLookup(stmt, map);
}

void TypeChecker::visitExprStmt(Maps *map, Node::Stmt *stmt) {
  auto expr_stmt = static_cast<ExprStmt *>(stmt);
  visitExpr(map, expr_stmt->expr);
}

void TypeChecker::visitProgram(Maps *map, Node::Stmt *stmt) {
  auto program_stmt = static_cast<ProgramStmt *>(stmt);
  for (auto &stmt : program_stmt->stmt) {
    visitStmt(map, stmt);
  }
}

void TypeChecker::visitConst(Maps *map, Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(map, const_stmt->value);
}

void TypeChecker::visitFn(Maps *map, Node::Stmt *stmt) {
  auto fn_stmt = static_cast<fnStmt *>(stmt);

  // add the function name and params the to the local table
  map->declare(map->local_symbol_table, fn_stmt->name, fn_stmt->returnType,
               fn_stmt->line, fn_stmt->pos);
  for (auto &param : fn_stmt->params) {
    map->declare(map->local_symbol_table, param.first, param.second,
                 fn_stmt->line, fn_stmt->pos);
  }

  visitStmt(map, fn_stmt->block);

  if (return_type != nullptr) {
    if (type_to_string(return_type) != type_to_string(fn_stmt->returnType)) {
      std::string msg = "Function '" + fn_stmt->name + "' must return a '" +
                        type_to_string(fn_stmt->returnType) + "' but got '" +
                        type_to_string(return_type) + "'";
      handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    }
  }

  // also add the function name to the global table and function table
  map->declare(map->global_symbol_table, fn_stmt->name, fn_stmt->returnType,
               fn_stmt->line, fn_stmt->pos);
  map->declare_fn(map, fn_stmt->name, {fn_stmt->name, fn_stmt->returnType},
                  fn_stmt->params, fn_stmt->line, fn_stmt->pos);

  return_type = nullptr;
  map->local_symbol_table
      .clear(); // clear the local table for the next function
}

void TypeChecker::visitBlock(Maps *map, Node::Stmt *stmt) {
  auto block_stmt = static_cast<BlockStmt *>(stmt);
  for (auto &stmt : block_stmt->stmts) {
    visitStmt(map, stmt);
  }
}

void TypeChecker::visitVar(Maps *map, Node::Stmt *stmt) {
  auto var_stmt = static_cast<VarStmt *>(stmt);

  if (map->local_symbol_table.empty()) {
    map->declare(map->global_symbol_table, var_stmt->name, var_stmt->type,
                 var_stmt->line, var_stmt->pos);
  }

  map->declare(map->local_symbol_table, var_stmt->name, var_stmt->type,
               var_stmt->line, var_stmt->pos);

  visitStmt(map, var_stmt->expr);

  if (return_type != nullptr) {
    if (type_to_string(return_type) != type_to_string(var_stmt->type)) {
      std::string msg = "Variable '" + var_stmt->name + "' must be a '" +
                        type_to_string(var_stmt->type) + "' but got '" +
                        type_to_string(return_type) + "'";
      handlerError(var_stmt->line, var_stmt->pos, msg, "", "Type Error");
    }
  }

  return_type = nullptr;
}

void TypeChecker::visitPrint(Maps *map, Node::Stmt *stmt) {
  auto print_stmt = static_cast<PrintStmt *>(stmt);
  for (auto &expr : print_stmt->args) {
    visitExpr(map, expr);
  }
}

void TypeChecker::visitReturn(Maps *map, Node::Stmt *stmt) {
  auto return_stmt = static_cast<ReturnStmt *>(stmt);
  visitExpr(map, return_stmt->expr);
}