#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"
#include "type.hpp"

#include <iostream>
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
    // check for a global variable declaration
    if (stmt->kind == NodeKind::ND_VAR_STMT) {
      auto var = static_cast<VarStmt *>(stmt);
      if (var->expr) {
        visitStmt(map, var);
      }
    }
    // make sure there are no loops or if statements in the global scope
    if (stmt->kind == NodeKind::ND_FOR_STMT ||
        stmt->kind == NodeKind::ND_WHILE_STMT ||
        stmt->kind == NodeKind::ND_IF_STMT) {
      std::string msg = "Loops and if statements are not allowed in the global "
                        "scope";
      handlerError(0, 0, msg, "", "Type Error");
    }
    visitStmt(map, stmt);
  }
}

void TypeChecker::visitConst(Maps *map, Node::Stmt *stmt) {
  auto const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(map, const_stmt->value);
}

void TypeChecker::visitFn(Maps *map, Node::Stmt *stmt) {
  auto fn_stmt = static_cast<FnStmt *>(stmt);

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

  if (return_type == nullptr) {
    // throw an error (but this should not happen ever)
    std::string msg = "return_type is not defined";
    handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
  }

  if (type_to_string(fn_stmt->returnType) == "void") {
    // also add the function name to the global table and function table
    map->declare(map->global_symbol_table, fn_stmt->name, fn_stmt->returnType,
                 fn_stmt->line, fn_stmt->pos);

    return_type = nullptr;
    map->local_symbol_table
        .clear(); // clear the local table for the next function
    return;
  }

  // Verify that we have a return stmt in the function
  if (!needsReturn && type_to_string(fn_stmt->returnType) != "void") {
    std::string msg = "Function '" + fn_stmt->name + "' requeries a return stmt "
                      "but none was found";
    handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    return;
  }

  if (type_to_string(return_type.get()) !=
      type_to_string(fn_stmt->returnType)) {
    std::string msg = "Function '" + fn_stmt->name + "' must return a '" +
                      type_to_string(fn_stmt->returnType) + "' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
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
  auto enum_stmt = static_cast<EnumStmt *>(stmt);
  // TODO: Implement the enum
  std::cerr << "Enums are not implemented yet in the TypeChecker!" << std::endl;
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
  if (return_stmt->expr == nullptr) {
    return;
  }
  needsReturn = true;

  visitExpr(map, return_stmt->expr);
}

void TypeChecker::visitTemplateStmt(Maps *map, Node::Stmt *stmt) {
  auto templateStmt = static_cast<TemplateStmt *>(stmt);

  // create a new type for the template which is an any type
  // auto type = new SymbolType("any");
  Node::Type *type = nullptr;

  // add the template to the global table
  for (auto &name : templateStmt->typenames) {
    type = new SymbolType(name);
    map->declare_fn(map, name, {name, type}, {}, templateStmt->line,
                    templateStmt->pos);
  }

  return_type = nullptr;
}

void TypeChecker::visitFor(Maps *map, Node::Stmt *stmt) {
  auto for_stmt = static_cast<ForStmt *>(stmt);

  // first we add the varName to the local table
  // the type of the for loop is the type of the for loop
  auto type = new SymbolType("int");
  map->declare(map->local_symbol_table, for_stmt->name,
               static_cast<Node::Type *>(type), for_stmt->line, for_stmt->pos);

  // now we visit the for loop and assign the type to the return type
  visitExpr(map, for_stmt->forLoop);

  visitExpr(map, for_stmt->condition);

  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "For loop condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(for_stmt->line, for_stmt->pos, msg, "", "Type Error");
  }

  // Now check if we have the increment in an optional parameter
  if (for_stmt->optional != nullptr) {
    visitExpr(map, for_stmt->optional);
  }

  // now we visit the block
  visitStmt(map, for_stmt->block);

  // clear the for loop var name
  map->local_symbol_table.erase(for_stmt->name);

  return_type = nullptr;
}

void TypeChecker::visitWhile(Maps *map, Node::Stmt *stmt) {
  auto while_stmt = static_cast<WhileStmt *>(stmt);

  // check the condition of the while loop
  visitExpr(map, while_stmt->condition);

  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "While loop condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(while_stmt->line, while_stmt->pos, msg, "", "Type Error");
  }

  // now we visit the block
  visitStmt(map, while_stmt->block);

  return_type = nullptr;
}

void TypeChecker::visitImport(Maps *map, Node::Stmt *stmt) {
  auto import_stmt = static_cast<ImportStmt *>(stmt);
  
  // store the current file name
  std::string file_name = node.current_file;
  node.current_file = import_stmt->name; // set the current file name to the import name
  
  // check if the import is already in the global table
  auto res = map->global_symbol_table.find(import_stmt->name);
  if (res != map->global_symbol_table.end()) {
    std::string msg = "'" + import_stmt->name + "' is already defined in the "
                      "global symbol table";
    handlerError(import_stmt->line, import_stmt->pos, msg, "", "Symbol Table Error");
  }

  // add the import to the global table
  map->declare(map->global_symbol_table, import_stmt->name, return_type.get(),
               import_stmt->line, import_stmt->pos);

  return_type = nullptr;
  node.current_file = file_name; // reset the current file name
}

void TypeChecker::visitLink(Maps *map, Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitExtern(Maps *map, Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitBreak(Maps *map, Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitContinue(Maps *map, Node::Stmt *stmt) {
  // nothing to do here
}