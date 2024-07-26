#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

void TypeChecker::visitStmt(global_symbol_table &gTable,
                            local_symbol_table &lTable,
                            function_table &fn_table, Node::Stmt *stmt) {
  StmtAstLookup(stmt, gTable, lTable, fn_table);
}

void TypeChecker::visitProgram(global_symbol_table &gTable,
                               local_symbol_table &lTable,
                               function_table &fn_table, Node::Stmt *stmt) {
  auto program_stmt = static_cast<ProgramStmt *>(stmt);
  for (auto &stmt : program_stmt->stmt) {
    visitStmt(gTable, lTable, fn_table, stmt);
  }
}

void TypeChecker::visitConst(global_symbol_table &gTable,
                             local_symbol_table &lTable,
                             function_table &fn_table, Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(gTable, lTable, fn_table, const_stmt->value);
}

// TODO: add a lookup for duplicate function names
void TypeChecker::visitFn(global_symbol_table &gTable,
                          local_symbol_table &lTable, function_table &fn_table,
                          Node::Stmt *stmt) {
  auto fn_stmt = static_cast<fnStmt *>(stmt);

  if (fn_stmt->name == "main") {
    foundMain = true;
    if (type_to_string(fn_stmt->returnType) != "int") {
      handlerError(fn_stmt->line, fn_stmt->pos,
                   "Main function must return an int", "", "Type Error");
    }
  }

  // add the function name and params the to the local table
  declare(lTable, fn_stmt->name, fn_stmt->returnType, fn_stmt->line,
          fn_stmt->pos);
  for (auto &param : fn_stmt->params) {
    declare(lTable, param.first, param.second, fn_stmt->line, fn_stmt->pos);
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
  declare(gTable, fn_stmt->name, fn_stmt->returnType, fn_stmt->line,
          fn_stmt->pos);
  declare_fn(fn_table, fn_stmt->name, {fn_stmt->name, fn_stmt->returnType},
             fn_stmt->params, fn_stmt->line, fn_stmt->pos);

  return_type = nullptr;
  lTable.clear(); // clear the local table for the next function

  for (auto &fn: fn_table) {
    std::cout << "Function: " << fn.first.first << " -> " << type_to_string(fn.first.second) << std::endl;
    for (auto &param: fn.second) {
      std::cout << "\tParam Name: " << param.first << std::endl;
      std::cout << "\tParam Type: " << type_to_string(param.second) << std::endl;
    }
  }
}

void TypeChecker::visitBlock(global_symbol_table &gTable,
                             local_symbol_table &lTable,
                             function_table &fn_table, Node::Stmt *stmt) {
  auto block_stmt = static_cast<BlockStmt *>(stmt);
  for (auto &stmt : block_stmt->stmts) {
    visitStmt(gTable, lTable, fn_table, stmt);
  }
}

void TypeChecker::visitReturn(global_symbol_table &gTable,
                              local_symbol_table &lTable,
                              function_table &fn_table, Node::Stmt *stmt) {
  auto return_stmt = static_cast<ReturnStmt *>(stmt);
  visitExpr(gTable, lTable, return_stmt->expr);
}