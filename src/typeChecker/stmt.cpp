#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

void TypeChecker::visitStmt(callables_table &ctable, symbol_table &table,
                            Node::Stmt *stmt) { 
  lookup(ctable, table, stmt);
}

void TypeChecker::visitExpr(callables_table &ctable, symbol_table &table,
                            Node::Expr *expr) {
  lookup(ctable, table, expr);
}

void TypeChecker::visitProgram(callables_table &ctable, symbol_table &table,
                               Node::Stmt *stmt) {
  auto program = static_cast<ProgramStmt *>(stmt);
  for (auto &stmt : program->stmt) {
    visitStmt(ctable, table, stmt);
  }
}

void TypeChecker::visitReturn(callables_table &ctable, symbol_table &table,
                              Node::Stmt *stmt) {
  auto return_stmt = static_cast<ReturnStmt *>(stmt);
  visitExpr(ctable, table, return_stmt->expr);
}

void TypeChecker::visitFn(callables_table &ctable, symbol_table &table,
                          Node::Stmt *stmt) {
  auto fn_stmt = static_cast<fnStmt *>(stmt);

  if (fn_stmt->name == "main") {
    foundMain = true;
    if(type_to_string(fn_stmt->returnType) != "int") {
      std::string msg = "Main function must return an int";
      handlerError(fn_stmt->line, fn_stmt->pos, msg, "");
    }
  }
  declare(table, fn_stmt->name, fn_stmt->returnType, fn_stmt->line, fn_stmt->pos);

  for (auto &param : fn_stmt->params) {
    declare(table, param.first, param.second, fn_stmt->line, fn_stmt->pos);
  }

  visitStmt(ctable, table, fn_stmt->block);

  // Now we need to check the return type of the function
  if (type_to_string(return_type) != type_to_string(fn_stmt->returnType)) {
    std::string msg = "Function '" + fn_stmt->name +
                      "' has a return type of '" + type_to_string(return_type) +
                      "' but expected '" + type_to_string(fn_stmt->returnType) +
                      "'";
    handlerError(fn_stmt->line, fn_stmt->pos, msg, "");
  }

  // add the function param and types to the callables table
  declare(ctable, fn_stmt->name, fn_stmt->params);

  // reset the return type
  return_type = nullptr;
}

void TypeChecker::visitBlock(callables_table &ctable, symbol_table &table,
                             Node::Stmt *stmt) {
  auto block_stmt = static_cast<BlockStmt *>(stmt);
  for (auto &stmt : block_stmt->stmts) {
    visitStmt(ctable, table, stmt);
  }
}

void TypeChecker::visitConst(callables_table &ctable, symbol_table &table,
                             Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);

  visitStmt(ctable, table, const_stmt->value);
}

void TypeChecker::visitVar(callables_table &ctable, symbol_table &table,
                           Node::Stmt *stmt) {
  auto var_stmt = static_cast<VarStmt *>(stmt);

  declare(table, var_stmt->name, var_stmt->type, var_stmt->line, var_stmt->pos);

  if (var_stmt->expr != nullptr) {
    visitStmt(ctable, table, var_stmt->expr);
    if (type_to_string(return_type) != type_to_string(var_stmt->type)) {
      std::string msg = "Variable '" + var_stmt->name + "' has a type of '" +
                        type_to_string(return_type) + "' but expected '" +
                        type_to_string(var_stmt->type) + "'";
      handlerError(var_stmt->line, var_stmt->pos, msg, "");
    }
  }
}

void TypeChecker::visitImport(callables_table &ctable, symbol_table &table,
                              Node::Stmt *stmt) {
  // NOTE: This works but for some reason when we build the line for the error
  // it doesn't show the correct line it shows the line from the main file
  auto import_stmt = static_cast<ImportStmt *>(stmt);
  
  auto current_file = node.current_file;
  node.current_file = import_stmt->name;

  visitStmt(ctable, table, import_stmt->stmt);

  node.current_file = current_file;
}