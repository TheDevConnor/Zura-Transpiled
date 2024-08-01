#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

void TypeChecker::visitStmt(Maps::global_symbol_table &gTable,
                            Maps::local_symbol_table &lTable,
                            Maps::function_table &fn_table, Node::Stmt *stmt) {
  StmtAstLookup(stmt, gTable, lTable, fn_table);
}

void TypeChecker::visitProgram(Maps::global_symbol_table &gTable,
                               Maps::local_symbol_table &lTable,
                               Maps::function_table &fn_table,
                               Node::Stmt *stmt) {
  auto program_stmt = static_cast<ProgramStmt *>(stmt);
  for (auto &stmt : program_stmt->stmt) {
    visitStmt(gTable, lTable, fn_table, stmt);
  }
}

void TypeChecker::visitConst(Maps::global_symbol_table &gTable,
                             Maps::local_symbol_table &lTable,
                             Maps::function_table &fn_table, Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(gTable, lTable, fn_table, const_stmt->value);
}

// TODO: add a lookup for duplicate function names
void TypeChecker::visitFn(Maps::global_symbol_table &gTable,
                          Maps::local_symbol_table &lTable,
                          Maps::function_table &fn_table, Node::Stmt *stmt) {
  auto fn_stmt = static_cast<fnStmt *>(stmt);

  if (fn_stmt->name == "main") {
    foundMain = true;
    if (type_to_string(fn_stmt->returnType) != "int") {
      handlerError(fn_stmt->line, fn_stmt->pos,
                   "Main function must return an int", "", "Type Error");
    }
  }

  // add the function name and params the to the local table
  Maps::declare(lTable, fn_stmt->name, fn_stmt->returnType, fn_stmt->line,
                fn_stmt->pos);
  for (auto &param : fn_stmt->params) {
    Maps::declare(lTable, param.first, param.second, fn_stmt->line,
                  fn_stmt->pos);
  }

  visitStmt(gTable, lTable, fn_table, fn_stmt->block);

  if (return_type != nullptr) {
    if (type_to_string(return_type) != type_to_string(fn_stmt->returnType)) {
      std::string msg = "Function '" + fn_stmt->name + "' must return a '" +
                        type_to_string(fn_stmt->returnType) + "' but got '" +
                        type_to_string(return_type) + "'";
      handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    }
  }

  // also add the function name to the global table and function table
  Maps::declare(gTable, fn_stmt->name, fn_stmt->returnType, fn_stmt->line,
                fn_stmt->pos);
  Maps::declare_fn(fn_table, fn_stmt->name,
                   {fn_stmt->name, fn_stmt->returnType}, fn_stmt->params,
                   fn_stmt->line, fn_stmt->pos);

  
  // print the Tables
  Maps::printTables(gTable, lTable, fn_table);


  return_type = nullptr;
  lTable.clear(); // clear the local table for the next function
}

void TypeChecker::visitBlock(Maps::global_symbol_table &gTable,
                             Maps::local_symbol_table &lTable,
                             Maps::function_table &fn_table, Node::Stmt *stmt) {
  auto block_stmt = static_cast<BlockStmt *>(stmt);
  for (auto &stmt : block_stmt->stmts) {
    visitStmt(gTable, lTable, fn_table, stmt);
  }
}

void TypeChecker::visitVar(Maps::global_symbol_table &gTable,
                           Maps::local_symbol_table &lTable,
                           Maps::function_table &fn_table, Node::Stmt *stmt) {
  auto var_stmt = static_cast<VarStmt *>(stmt);

  Maps::declare(lTable, var_stmt->name, var_stmt->type, var_stmt->line,
                var_stmt->pos);

  visitStmt(gTable, lTable, fn_table, var_stmt->expr);

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

void TypeChecker::visitReturn(Maps::global_symbol_table &gTable,
                              Maps::local_symbol_table &lTable,
                              Maps::function_table &fn_table,
                              Node::Stmt *stmt) {
  auto return_stmt = static_cast<ReturnStmt *>(stmt);
  visitExpr(gTable, lTable, fn_table, return_stmt->expr);
}