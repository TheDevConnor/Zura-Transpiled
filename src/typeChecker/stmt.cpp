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
  ExprStmt *expr_stmt = static_cast<ExprStmt *>(stmt);
  visitExpr(map, expr_stmt->expr);
}

void TypeChecker::visitProgram(Maps *map, Node::Stmt *stmt) {
  ProgramStmt *program_stmt = static_cast<ProgramStmt *>(stmt);
  for (Node::Stmt *stmt : program_stmt->stmt) {
    switch (stmt->kind) {
    case NodeKind::ND_VAR_STMT: {
      VarStmt *var = static_cast<VarStmt *>(stmt);
      if (var->expr == nullptr) {
        std::string msg = "Global variables must be initialized";
        handlerError(var->line, var->pos, msg, "", "Type Error");
      }
      break;
    }
    case NodeKind::ND_FOR_STMT:
    case NodeKind::ND_WHILE_STMT:
    case NodeKind::ND_IF_STMT: {
      std::string msg = "Loops and if statements are not allowed in the global "
                        "scope";
      handlerError(0, 0, msg, "", "Type Error");
      break;
    }
    default:
      visitStmt(map, stmt);
      break;
    }
  }
}

void TypeChecker::visitConst(Maps *map, Node::Stmt *stmt) {
  ConstStmt *const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(map, const_stmt->value);
}

void TypeChecker::visitFn(Maps *map, Node::Stmt *stmt) {
  FnStmt *fn_stmt = static_cast<FnStmt *>(stmt);

  // add the function name and params the to the local table
  declare(map->local_symbol_table, fn_stmt->name, fn_stmt->returnType,
          fn_stmt->line, fn_stmt->pos);
  for (std::pair<std::string, Node::Type *> &param : fn_stmt->params) {
    declare(map->local_symbol_table, param.first, param.second, fn_stmt->line,
            fn_stmt->pos);
  }
  
  declare_fn(map, {fn_stmt->name, fn_stmt->returnType},
                 fn_stmt->params, fn_stmt->line, fn_stmt->pos);

  visitStmt(map, fn_stmt->block);

  if (return_type == nullptr) {
    // throw an error (but this should not happen ever)
    std::string msg = "return_type is not defined";
    handlerError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
  }

  if (type_to_string(fn_stmt->returnType) == "void") {
    // also add the function name to the global table and function table
    declare(map->global_symbol_table, fn_stmt->name, fn_stmt->returnType,
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

  std::string msg = "Function '" + fn_stmt->name +
                    "' requeries a retrun type of '" +
                    type_to_string(fn_stmt->returnType) + "' but got '" +
                    type_to_string(return_type.get()) + "' instead.";
  auto check = checkTypeMatch(
      std::make_shared<SymbolType>(type_to_string(fn_stmt->returnType)),
      std::make_shared<SymbolType>(type_to_string(return_type.get())),
      fn_stmt->name, fn_stmt->line, fn_stmt->pos, msg);
  if (!check) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  // also add the function name to the global table and function table
  declare(map->global_symbol_table, fn_stmt->name, fn_stmt->returnType,
               fn_stmt->line, fn_stmt->pos);

  return_type = nullptr;
  map->local_symbol_table
      .clear(); // clear the local table for the next function
}

void TypeChecker::visitBlock(Maps *map, Node::Stmt *stmt) {
  BlockStmt *block_stmt = static_cast<BlockStmt *>(stmt);
  for (Node::Stmt *stmt : block_stmt->stmts) {
    visitStmt(map, stmt);
  }
}

void TypeChecker::visitStruct(Maps *map, Node::Stmt *stmt) {
  StructStmt *struct_stmt = static_cast<StructStmt *>(stmt);
  SymbolType *type = new SymbolType("struct");

  // add the struct name to the local table and global table
  declare(map->local_symbol_table, struct_stmt->name,
               static_cast<Node::Type *>(type), struct_stmt->line,
               struct_stmt->pos);
  declare(map->global_symbol_table, struct_stmt->name,
               static_cast<Node::Type *>(type), struct_stmt->line,
               struct_stmt->pos);
  
  // visit the fields Aka the variables
  for (std::pair<std::string, Node::Type *> &field : struct_stmt->fields) {
    // add the field to the struct table
    map->struct_table[struct_stmt->name].push_back({field.first, field.second});
  }

  // TODO: Handle functions in structs

  return_type = std::make_shared<SymbolType>("struct");
}

void TypeChecker::visitEnum(Maps *map, Node::Stmt *stmt) {
  EnumStmt *enum_stmt = static_cast<EnumStmt *>(stmt);
  SymbolType *type = new SymbolType("enum");
  
  // add the enum name to the local table and global table
  declare(map->local_symbol_table, enum_stmt->name,
               static_cast<Node::Type *>(type), enum_stmt->line,
               enum_stmt->pos);
  declare(map->global_symbol_table, enum_stmt->name,
                static_cast<Node::Type *>(type), enum_stmt->line,
                enum_stmt->pos);

  if (enum_stmt->fields.empty()) {
    std::string msg = "Enum '" + enum_stmt->name + "' must have at least one field";
    handlerError(enum_stmt->line, enum_stmt->pos, msg, "", "Type Error");
  }

  // visit the fields Aka the variables
  for (int i = 0; i < enum_stmt->fields.size(); i++) {
    declare(map->local_symbol_table, enum_stmt->fields[i],
            static_cast<Node::Type *>(new SymbolType("int")), enum_stmt->line,
            enum_stmt->pos);
    // add the field to the struct table
    map->enum_table[enum_stmt->name].push_back({enum_stmt->fields[i], i});
  }
  
  return_type = std::make_shared<SymbolType>("enum");
}

void TypeChecker::visitIf(Maps *map, Node::Stmt *stmt) {
  IfStmt *if_stmt = static_cast<IfStmt *>(stmt);
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
  VarStmt *var_stmt = static_cast<VarStmt *>(stmt);

  declare(map->local_symbol_table, var_stmt->name, var_stmt->type,
               var_stmt->line, var_stmt->pos);

  // check if the type is a struct or enum
  if (map->struct_table.find(type_to_string(var_stmt->type)) !=
      map->struct_table.end()) {
    return_type = std::make_shared<SymbolType>(type_to_string(var_stmt->type));
  } else if (map->enum_table.find(type_to_string(var_stmt->type)) !=
             map->enum_table.end()) {
    return_type = std::make_shared<SymbolType>(type_to_string(var_stmt->type));
  }

  // check if the variable is initialized
  if (var_stmt->expr == nullptr) {
    return;
  }
  visitExpr(map, var_stmt->expr);

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

// Why, oh why, C++, is this the solution?
Node::Type *createDuplicate(Node::Type *original) {
  if (original->kind == NodeKind::ND_SYMBOL_TYPE) {
    SymbolType *sym = static_cast<SymbolType *>(original);
    return new SymbolType(sym->name);
  } else if (original->kind == NodeKind::ND_ARRAY_TYPE) {
    ArrayType *arr = static_cast<ArrayType *>(original);
    return new ArrayType(createDuplicate(arr->underlying));
  } else if (original->kind == NodeKind::ND_POINTER_TYPE) {
    PointerType *ptr = static_cast<PointerType *>(original);
    return new PointerType(ptr->pointer_type, createDuplicate(ptr->underlying));
  }
  return nullptr;
}

void TypeChecker::visitPrint(Maps *map, Node::Stmt *stmt) {
  PrintStmt *print_stmt = static_cast<PrintStmt *>(stmt);
  for (int i = 0; i < print_stmt->args.size(); i++) {
    visitExpr(map, print_stmt->args[i]);
    print_stmt->args[i]->asmType = createDuplicate(return_type.get());
  }
  return_type = nullptr;
}

void TypeChecker::visitReturn(Maps *map, Node::Stmt *stmt) {
  ReturnStmt *return_stmt = static_cast<ReturnStmt *>(stmt);
  if (return_stmt->expr == nullptr) {
    return;
  }
  needsReturn = true;

  visitExpr(map, return_stmt->expr);
}

void TypeChecker::visitTemplateStmt(Maps *map, Node::Stmt *stmt) {
  TemplateStmt *templateStmt = static_cast<TemplateStmt *>(stmt);
  // template <typename T>
  std::cout << "Not yet Implemented! Templates" << std::endl;
  return_type = nullptr;
}

void TypeChecker::visitFor(Maps *map, Node::Stmt *stmt) {
  ForStmt *for_stmt = static_cast<ForStmt *>(stmt);

  // first we add the varName to the local table
  // the type of the for loop is the type of the for loop
  SymbolType *type = new SymbolType("int");
  declare(map->local_symbol_table, for_stmt->name,
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
  WhileStmt *while_stmt = static_cast<WhileStmt *>(stmt);

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
  ImportStmt *import_stmt = static_cast<ImportStmt *>(stmt);
  
  // store the current file name
  std::string file_name = node.current_file;
  node.current_file = import_stmt->name; // set the current file name to the import name
  
  // check if the import is already in the global table
  std::unordered_map<std::string, Node::Type *>::iterator res = map->global_symbol_table.find(import_stmt->name);
  if (res != map->global_symbol_table.end()) {
    std::string msg = "'" + import_stmt->name + "' is already defined in the "
                      "global symbol table";
    handlerError(import_stmt->line, import_stmt->pos, msg, "", "Symbol Table Error");
  }

  // add the import to the global table
  declare(map->global_symbol_table, import_stmt->name, return_type.get(),
               import_stmt->line, import_stmt->pos);

  // type check the import
  visitStmt(map, import_stmt->stmt);

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