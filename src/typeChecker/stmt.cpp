#include "../ast/stmt.hpp"
#include "../ast/types.hpp"
#include "../helper/error/error.hpp"
#include "type.hpp"
#include "typeMaps.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

void TypeChecker::visitStmt(Node::Stmt *stmt) {
  StmtAstLookup(stmt);
}

void TypeChecker::visitExprStmt(Node::Stmt *stmt) {
  ExprStmt *expr_stmt = static_cast<ExprStmt *>(stmt);
  visitExpr(expr_stmt->expr);
}

void TypeChecker::visitProgram(Node::Stmt *stmt) {
  ProgramStmt *program_stmt = static_cast<ProgramStmt *>(stmt);
  for (Node::Stmt *stmt : program_stmt->stmt) {
    switch (stmt->kind) {
    case NodeKind::ND_VAR_STMT: {
      VarStmt *var = static_cast<VarStmt *>(stmt);
      if (var->expr == nullptr) {
        std::string msg = "Global variables must be initialized";
        handleError(var->line, var->pos, msg, "", "Type Error");
      }
      break;
    }
    case NodeKind::ND_FOR_STMT:
    case NodeKind::ND_WHILE_STMT:
    case NodeKind::ND_IF_STMT: {
      std::string msg = "Loops and if statements are not allowed in the global "
                        "scope";
      handleError(0, 0, msg, "", "Type Error");
      break;
    }
    default:
      visitStmt(stmt);
      break;
    }
  }
}

void TypeChecker::visitConst(Node::Stmt *stmt) {
  ConstStmt *const_stmt = static_cast<ConstStmt *>(stmt);
  visitStmt(const_stmt->value);
}

void TypeChecker::visitFn(Node::Stmt *stmt) {
  FnStmt *fn_stmt = static_cast<FnStmt *>(stmt);

  // Declare function in local table
  context->declareLocal(fn_stmt->name, fn_stmt->returnType);

  // Debugging: Ensure function is added
  if (context->lookup(fn_stmt->name) != fn_stmt->returnType) {
      std::cerr << "Error: Function not properly declared!" << std::endl;
      return;
  }

  // Add function parameters
  std::unordered_map<std::string, Node::Type *> params;
  for (std::pair<std::string, Node::Type *> &param : fn_stmt->params) {
      if (!param.second) {
          std::cerr << "Error: Parameter " << param.first << " has nullptr type!" << std::endl;
          continue;
      }
      params[param.first] = param.second;
      context->declareLocal(param.first, param.second);
  }

  // Add function to functionTable
  context->functionTable.declare(fn_stmt->name, params, fn_stmt->returnType);

  visitStmt(fn_stmt->block);

  // Ensure return type is properly checked
  if (return_type == nullptr) {
      std::cerr << "Error: Return type is not defined!" << std::endl;
      handleError(fn_stmt->line, fn_stmt->pos, "Return type is not defined", "", "Type Error");
  }

  // Check return statement requirements
  if (!needsReturn && type_to_string(fn_stmt->returnType) != "void") {
      std::cerr << "Error: Function requires return statement but none found!" << std::endl;
      handleError(fn_stmt->line, fn_stmt->pos, "Function requires a return statement", "", "Type Error");
      return;
  }

  // Ensure function return type matches expected type
  std::string expectedType = type_to_string(fn_stmt->returnType);
  std::string actualType = type_to_string(return_type.get());

  std::string msg = "Function '" + fn_stmt->name + "' requeries a return type of '" + type_to_string(fn_stmt->returnType) + "' but got '" +
                    type_to_string(return_type.get()) + "' instead.";
  bool check = checkTypeMatch(
      std::make_shared<SymbolType>(type_to_string(fn_stmt->returnType)),
      std::make_shared<SymbolType>(type_to_string(return_type.get())),
      fn_stmt->name, fn_stmt->line, fn_stmt->pos, msg);
  if (!check) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  // Declare function in global scope
  context->declareGlobal(fn_stmt->name, fn_stmt->returnType);

  return_type = nullptr;
  context->exitScope(); // Clear local table for next function
}

void TypeChecker::visitBlock(Node::Stmt *stmt) {
  BlockStmt *block_stmt = static_cast<BlockStmt *>(stmt);
  for (Node::Stmt *stmt : block_stmt->stmts) {
    visitStmt(stmt);
  }
}

void TypeChecker::visitStruct(Node::Stmt *stmt) {
  StructStmt *struct_stmt = static_cast<StructStmt *>(stmt);
  SymbolType *type = new SymbolType(struct_stmt->name);

  // add the struct name to the local table and global table
  context->declareLocal(struct_stmt->name, static_cast<Node::Type *>(type));
  context->declareGlobal(struct_stmt->name, static_cast<Node::Type *>(type));


  context->structTable.insert({struct_stmt->name, {}}); // Declare blank and insert later

  // visit the fields Aka the variables
  for (std::pair<std::string, Node::Type *> &field : struct_stmt->fields) {
    context->structTable.addMember(struct_stmt->name, field.first, field.second);
  }

  // handle fn stmts in the struct
  std::unordered_map<std::string, Node::Type *> params;
  for (Node::Stmt *stmt : struct_stmt->stmts) {
    FnStmt *fn_stmt = static_cast<FnStmt *>(stmt);

    for (std::pair<std::string, Node::Type *> &param : fn_stmt->params) {
      // add the params to the local table
      params[param.first] = param.second;
      context->declareLocal(param.first, param.second);
    }

    // declare_struct_fn(map, {fn_stmt->name, fn_stmt->returnType}, params,
    //                   fn_stmt->line, fn_stmt->pos, struct_stmt->name);
    context->structTable.addFunction(struct_stmt->name, fn_stmt->name, fn_stmt->returnType, params);

    visitStmt(fn_stmt->block);
    params.clear();

    if (type_to_string(fn_stmt->returnType) == "void") {
      return_type = nullptr;
      context->exitScope(); // clear the local table for the next function
      continue;
    }

    if (return_type == nullptr) {
      // throw an error (but this should not happen ever)
      std::string msg = "Struct return type is not defined";
      handleError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    }

    // Verify that we have a return stmt in the function
    if (!needsReturn && type_to_string(fn_stmt->returnType) != "void") {
      std::string msg = "Function '" + fn_stmt->name +
                        "' requeries a return stmt "
                        "but none was found";
      handleError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
      return;
    }

    std::string msg = "Function '" + fn_stmt->name +
                      "' requeries a return type of '" +
                      type_to_string(fn_stmt->returnType) + "' but got '" +
                      type_to_string(return_type.get()) + "' instead.";
    bool check = checkTypeMatch(
        std::make_shared<SymbolType>(type_to_string(fn_stmt->returnType)),
        std::make_shared<SymbolType>(type_to_string(return_type.get())),
        fn_stmt->name, fn_stmt->line, fn_stmt->pos, msg);
    if (!check) {
      return_type = std::make_shared<SymbolType>("unknown");
      return;
    }

    // also add the function name to the global table and function table
    context->declareGlobal(fn_stmt->name, fn_stmt->returnType);

    return_type = nullptr;
    context->exitScope(); // clear the local table for the next function
  }

  return_type = std::make_shared<SymbolType>(struct_stmt->name);
}

void TypeChecker::visitEnum(Node::Stmt *stmt) {
  EnumStmt *enum_stmt = static_cast<EnumStmt *>(stmt);
  SymbolType *type = new SymbolType("enum");

  // add the enum name to the local table and global table
  context->declareLocal(enum_stmt->name, static_cast<Node::Type *>(type));
  context->declareGlobal(enum_stmt->name, static_cast<Node::Type *>(type));

  if (enum_stmt->fields.empty()) {
    std::string msg = "Enum '" + enum_stmt->name + "' must have at least one field";
    handleError(enum_stmt->line, enum_stmt->pos, msg, "", "Type Error");
  }

  // visit the fields Aka the variables
  for (size_t i = 0; i < enum_stmt->fields.size(); i++) {
    context->enumTable.addMember(enum_stmt->name, enum_stmt->fields[i], i);
    context->declareLocal(enum_stmt->fields[i], static_cast<Node::Type *>(new SymbolType("int")));
  }

  // set return type to the name of the enum
  return_type = std::make_shared<SymbolType>(enum_stmt->name);
}

void TypeChecker::visitIf(Node::Stmt *stmt) {
  IfStmt *if_stmt = static_cast<IfStmt *>(stmt);
  visitExpr(if_stmt->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "If condition must be a 'bool' but got '" + type_to_string(return_type.get()) + "'";
    handleError(if_stmt->line, if_stmt->pos, msg, "", "Type Error");
  }
  visitStmt(if_stmt->thenStmt);
  if (if_stmt->elseStmt != nullptr) {
    visitStmt(if_stmt->elseStmt);
  }
}

void TypeChecker::visitVar(Node::Stmt *stmt) {
  VarStmt *var_stmt = static_cast<VarStmt *>(stmt);

  context->declareLocal(var_stmt->name, var_stmt->type);

  // check if the type is a struct or enum
  if (context->structTable.contains(type_to_string(var_stmt->type)) ||
      context->enumTable.contains(type_to_string(var_stmt->type))) {
    return_type = std::make_shared<SymbolType>(type_to_string(var_stmt->type));
  }

  // Now check if we have a template struct
  if (var_stmt->type->kind == ND_TEMPLATE_STRUCT_TYPE) {
    TemplateStructType *temp = static_cast<TemplateStructType *>(var_stmt->type);

    // auto res = map->struct_table.find(type_to_string(temp->name));
    auto res = context->structTable.contains(type_to_string(temp->name));
    if (!res) {
      std::string msg = "Template struct '" + type_to_string(temp->name) + "' is not defined";
      handleError(var_stmt->line, var_stmt->pos, msg, "", "Type Error");
    }

    // If we found the struct now lets go through the fields and find the value
    for (auto &field : context->structTable.at(type_to_string(temp->name))) {
      // declare the field in the local table
      context->declareLocal(field.first, field.second.first);
    }

    // Now we can set the return type to the underlying type
    return_type = std::make_shared<SymbolType>(type_to_string(temp->underlying));

    // Now we can set the type of the variable to the underlying type
    var_stmt->type = temp->underlying;
  } else if (var_stmt->expr ==
             nullptr) { // check if the variable is initialized
    return;
  } else if (var_stmt->type->kind == ND_ARRAY_TYPE) {
    // Set the expr.type to the type of the variable so it can be used
    // in codegen process.
    ArrayExpr *array_expr = static_cast<ArrayExpr *>(var_stmt->expr);
    ArrayType *array_type = static_cast<ArrayType *>(var_stmt->type);
    if (array_type->constSize < 1) {
      // If the array was declared but its type was []
      // we assume the constSize is the size of the array expr
      var_stmt->type =
          new ArrayType(array_type->underlying, array_expr->elements.size());
    } else if(var_stmt->expr->kind == NodeKind::ND_ARRAY) { // auto filled arrays will always have 1 element (the one to autofill) so do not error in that case
      // If the array was declared with a size we need to check if the size
      // of the array expr is the same as the size of the array type
      if ((size_t)array_type->constSize != array_expr->elements.size()) {
        std::string msg = "Array '" + var_stmt->name + "' requires " +
                          std::to_string(array_type->constSize) +
                          " elements but got " +
                          std::to_string(array_expr->elements.size());
        handleError(var_stmt->line, var_stmt->pos, msg, "", "Type Error");
      }
    }
    array_expr->type =
        new ArrayType(array_type->underlying, array_expr->elements.size());
    return_type = std::make_shared<ArrayType>(array_type->underlying,
                                              array_type->constSize);
    visitExpr(var_stmt->expr); // Visit the array, and by extension, its elements
    return_type = std::make_shared<ArrayType>(array_type->underlying,
                                              array_type->constSize);
  } else if (var_stmt->type->kind == ND_POINTER_TYPE) {
    PointerType *ptr = static_cast<PointerType *>(var_stmt->type);
    if (var_stmt->expr->kind == NodeKind::ND_NULL) {
      return_type = std::make_shared<PointerType>(ptr->underlying);
    } else {
      visitExpr(var_stmt->expr);
      return_type = std::make_shared<PointerType>(ptr->underlying);
    }
  } else {
    visitExpr(var_stmt->expr);
  }

  // check if the variable type is the same as the expr type
  std::string msg = "Variable '" + var_stmt->name + "' requires type '" +
                    type_to_string(var_stmt->type) + "' but got '" +
                    type_to_string(return_type.get()) + "'";
  checkTypeMatch(
      std::make_shared<SymbolType>(type_to_string(var_stmt->type)),
      std::make_shared<SymbolType>(type_to_string(return_type.get())),
      var_stmt->name, var_stmt->line, var_stmt->pos, msg);

  return_type = nullptr;
}

void TypeChecker::visitPrint(Node::Stmt *stmt) {
  OutputStmt *print_stmt = static_cast<OutputStmt *>(stmt);
  visitExpr(print_stmt->fd);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Print requires the file descriptor to be of type 'int' "
                      "but got '" + type_to_string(return_type.get()) + "'";
    handleError(print_stmt->line, print_stmt->pos, msg, "", "Type Error");
  }

  // check if we are a println statement if so add a new line to the end
  if (print_stmt->isPrintln) print_stmt->args.push_back(new StringExpr(0, 0, "'\n'", 0));

  for (size_t i = 0; i < print_stmt->args.size(); i++) {
    visitExpr(print_stmt->args[i]);
    print_stmt->args[i]->asmType = createDuplicate(return_type.get());
  }

  return_type = nullptr;
}

void TypeChecker::visitReturn(Node::Stmt *stmt) {
  ReturnStmt *return_stmt = static_cast<ReturnStmt *>(stmt);
  if (return_stmt->expr == nullptr) {
    return;
  }
  needsReturn = true;

  visitExpr(return_stmt->expr);
}

void TypeChecker::visitTemplateStmt(Node::Stmt *stmt) {
  TemplateStmt *templateStmt = static_cast<TemplateStmt *>(stmt);
  // template <T>
  // add to the template table
  // map->template_table = templateStmt->typenames;

  // // add the template to the global table
  // for (std::string &name : templateStmt->typenames) {
  //   SymbolType *type = new SymbolType("any");
  //   declare(map->global_symbol_table, name, static_cast<Node::Type *>(type),
  //           templateStmt->line, templateStmt->pos);
  // }
  std::cout << "TemplateStmt not implemented yet" << std::endl;
  exit(1);
}

void TypeChecker::visitFor(Node::Stmt *stmt) {
  ForStmt *for_stmt = static_cast<ForStmt *>(stmt);

  // create a new scope for the for loop
  context->enterScope();

  // first we add the varName to the local table
  // the type of the for loop is the type of the for loop
  SymbolType *type = new SymbolType("int");
  // declare(map->local_symbol_table, for_stmt->name,
  //         static_cast<Node::Type *>(type), for_stmt->line, for_stmt->pos);
  context->declareLocal(for_stmt->name, static_cast<Node::Type *>(type));

  // now we visit the for loop and assign the type to the return type
  visitExpr(for_stmt->forLoop);

  visitExpr(for_stmt->condition);

  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "For loop condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(for_stmt->line, for_stmt->pos, msg, "", "Type Error");
  }

  // Now check if we have the increment in an optional parameter
  if (for_stmt->optional != nullptr) {
    visitExpr(for_stmt->optional);
  }

  // now we visit the block
  visitStmt(for_stmt->block);

  // clear the for loop var name from the local table
  context->exitScope(); // clear the local table for the next function

  return_type = nullptr;
}

void TypeChecker::visitWhile(Node::Stmt *stmt) {
  WhileStmt *while_stmt = static_cast<WhileStmt *>(stmt);

  // check the condition of the while loop
  visitExpr(while_stmt->condition);

  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "While loop condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(while_stmt->line, while_stmt->pos, msg, "", "Type Error");
  }

  if (while_stmt->optional != nullptr) {
    visitExpr(while_stmt->optional);
  }

  // now we visit the block
  visitStmt(while_stmt->block);

  return_type = nullptr;
}

void TypeChecker::visitImport(Node::Stmt *stmt) {
  ImportStmt *import_stmt = static_cast<ImportStmt *>(stmt);

  // store the current file name
  std::string file_name = node.current_file;
  node.current_file =
      import_stmt->name; // set the current file name to the import name

  // check if the import is already in the global table
  // std::unordered_map<std::string, Node::Type *>::iterator res = map->global_symbol_table.find(import_stmt->name);
  // if (res != map->global_symbol_table.end()) {
  //   std::string msg = "'" + import_stmt->name + "' has already been imported.";
  //   handleError(import_stmt->line, import_stmt->pos, msg, "", "Type Error");
  // }
  std::unordered_map<std::string, Node::Type *>::iterator res = context->globalSymbols.find(import_stmt->name);
  if (res != context->globalSymbols.end()) {
    std::string msg = "'" + import_stmt->name + "' has already been imported.";
    handleError(import_stmt->line, import_stmt->pos, msg, "", "Type Error");
  }

  // add the import to the global table
  // declare(map->global_symbol_table, import_stmt->name, return_type.get(),
  //         import_stmt->line, import_stmt->pos);
  context->declareGlobal(import_stmt->name, return_type.get());

  // type check the import
  visitStmt(import_stmt->stmt);

  return_type = nullptr;
  node.current_file = file_name; // reset the current file name
}

void TypeChecker::visitMatch(Node::Stmt *stmt) {
  MatchStmt *match_stmt = static_cast<MatchStmt *>(stmt);
  visitExpr(match_stmt->coverExpr);

  for (std::pair<Node::Expr *, Node::Stmt *> &pair : match_stmt->cases) {
    visitExpr(pair.first); // Implement actual jump tables and math later. For now, all return_types for case expressions are allowed.
    visitStmt(pair.second);
  }

  return_type = nullptr;
}

void TypeChecker::visitLink(Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitExtern(Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitBreak(Node::Stmt *stmt) {
  // nothing to do here
}

void TypeChecker::visitContinue(Node::Stmt *stmt) {
  // nothing to do here
}

//              varName,     bytes
// @input( (char* | []char),  int)
void TypeChecker::visitInput(Node::Stmt *stmt) {
  InputStmt *input_stmt = static_cast<InputStmt *>(stmt);

  visitExpr(input_stmt->fd);
  if (type_to_string(return_type.get()) != "int") {
    std::string msg = "Input system call fd must be a 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  visitExpr(input_stmt->bufferOut); // str, char*, char[]
  if (type_to_string(return_type.get()) != "str" &&
      type_to_string(return_type.get()) != "*char" &&
      type_to_string(return_type.get()) != "[]char") {
    std::string msg = "Input variable name must be a 'string', 'char*' or 'char[]' but got '" + type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  visitExpr(input_stmt->maxBytes);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Input max bytes must be a 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  return_type = nullptr;
}

void TypeChecker::visitClose(Node::Stmt *stmt) {
  CloseStmt *close_stmt = static_cast<CloseStmt *>(stmt);
  visitExpr(close_stmt->fd);
  if (type_to_string(return_type.get()) != "int") {
    std::string msg = "Close system call fd must be a 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(close_stmt->line, close_stmt->pos, msg, "", "Type Error");
  }
  return_type = nullptr;
}