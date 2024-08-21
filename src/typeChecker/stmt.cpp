#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

#include <memory>

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

  map->declare_fn(map, fn_stmt->name, {fn_stmt->name, fn_stmt->returnType},
                  fn_stmt->params, fn_stmt->line, fn_stmt->pos);

  visitStmt(map, fn_stmt->block);

  if (return_type != nullptr) {
    if (type_to_string(return_type.get()) != type_to_string(fn_stmt->returnType)) {
      std::string msg = "Function '" + fn_stmt->name + "' must return a '" +
                        type_to_string(fn_stmt->returnType) + "' but got '" +
                        type_to_string(return_type.get()) + "'";
      handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    }
  }

  // also add the function name to the global table and function table
  map->declare(map->global_symbol_table, fn_stmt->name, fn_stmt->returnType,
               fn_stmt->line, fn_stmt->pos);

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

void TypeChecker::visitStruct(Maps *map, Node::Stmt *stmt) {
  auto struct_stmt = static_cast<StructStmt *>(stmt);

  // Declare the struct name as a type and add it to the global table
  Node::Type *struct_type = new SymbolType(struct_stmt->name);

  // Add the struct fields to the local table of the struct
  for (auto &field : struct_stmt->fields) {
    map->declare(map->local_symbol_table, field.first, field.second,
                 struct_stmt->line, struct_stmt->pos);
  }

  // Add the struct to the global table
  map->declare(map->global_symbol_table, struct_stmt->name, struct_type,
               struct_stmt->line, struct_stmt->pos);

  // Add the struct to the function table
  map->declare_fn(map, struct_stmt->name, {struct_stmt->name, struct_type},
                  struct_stmt->fields, struct_stmt->line, struct_stmt->pos);

  return_type = nullptr;
  map->local_symbol_table.clear();
}

void TypeChecker::visitEnum(Maps *map, Node::Stmt *stmt) {
  std::cout << "Not implemented yet" << std::endl;
}

void TypeChecker::visitIf(Maps *map, Node::Stmt *stmt) {
  auto if_stmt = static_cast<IfStmt *>(stmt);
  visitExpr(map, if_stmt->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "If condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(if_stmt->line, if_stmt->pos, msg, "", "Type Error");
  }
  visitStmt(map, if_stmt->thenStmt);
  if (if_stmt->elseStmt != nullptr) {
    visitStmt(map, if_stmt->elseStmt);
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

  // check if the variable is initialized
  if (var_stmt->expr == nullptr) {
    return;
  }
  visitStmt(map, var_stmt->expr);

  if (return_type != nullptr) {
    if (type_to_string(return_type.get()) != type_to_string(var_stmt->type)) {
      std::string msg = "Variable '" + var_stmt->name + "' must be a '" +
                        type_to_string(var_stmt->type) + "' but got '" +
                        type_to_string(return_type.get()) + "'";
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

  if (return_stmt->stmt != nullptr) { // if the return statement is a statement
    visitStmt(map, return_stmt->stmt);
    return;
  }

  visitExpr(map, return_stmt->expr);
}

void TypeChecker::visitTemplateStmt(Maps *map, Node::Stmt *stmt) {
  auto templateStmt = static_cast<TemplateStmt *>(stmt);

  // create a new type for the template which is an any type
  auto type = new SymbolType("any");
  
  // add the template to the global table
  for (auto &name : templateStmt->typenames) {
    map->declare_fn(map, name, {name, type}, {}, templateStmt->line, templateStmt->pos);
  }

  return_type = nullptr;
}
