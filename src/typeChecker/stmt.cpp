#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

void TypeChecker::visitStmt(global_symbol_table &gTable,
                            local_symbol_table &lTable, Node::Stmt *stmt) {
  StmtAstLookup(stmt, gTable, lTable);
}

void TypeChecker::visitProgram(global_symbol_table &gTable,
                               local_symbol_table &lTable, Node::Stmt *stmt) {
  auto program_stmt = static_cast<ProgramStmt *>(stmt);
  for (auto &stmt : program_stmt->stmt) {
    visitStmt(gTable, lTable, stmt);
  }
}

void TypeChecker::visitConst(global_symbol_table &gTable,
                             local_symbol_table &lTable, Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(gTable, lTable, const_stmt->value);
}

void TypeChecker::visitFn(global_symbol_table &gTable,
                          local_symbol_table &lTable, Node::Stmt *stmt) {
  auto fn_stmt = static_cast<fnStmt *>(stmt);

  if (fn_stmt->name == "main") {
    foundMain = true;
    if (type_to_string(fn_stmt->returnType) != "int") {
      handlerError(fn_stmt->line, fn_stmt->pos,
                   "Main function must return an int", "", "Type Error");
    }
  }

  declare(gTable, fn_stmt->name, fn_stmt->returnType, fn_stmt->line,
          fn_stmt->pos);
  for (auto &param : fn_stmt->params) {
    declare(lTable, param.first, param.second, fn_stmt->line, fn_stmt->pos);
  }

  visitStmt(gTable, lTable, fn_stmt->block);

  if (return_type != nullptr) {
    if (type_to_string(return_type) != type_to_string(fn_stmt->returnType)) {
      std::string msg = "Function '" + fn_stmt->name + "' must return a '" +
                type_to_string(fn_stmt->returnType) + "' but got '" +
                type_to_string(return_type) + "'";
      handlerError(fn_stmt->line, fn_stmt->pos,
                   msg, "", "Type Error");
    }
  }

  return_type = nullptr;
  lTable.clear(); // clear the local table for the next function
  // TODO: push to the function table for later use
}

void TypeChecker::visitBlock(global_symbol_table &gTable,
                             local_symbol_table &lTable, Node::Stmt *stmt) {
  auto block_stmt = static_cast<BlockStmt *>(stmt);
  for (auto &stmt : block_stmt->stmts) {
    visitStmt(gTable, lTable, stmt);
  }
}

void TypeChecker::visitReturn(global_symbol_table &gTable,
                              local_symbol_table &lTable, Node::Stmt *stmt) {
  auto return_stmt = static_cast<ReturnStmt *>(stmt);
  visitExpr(gTable, lTable, return_stmt->expr);
}