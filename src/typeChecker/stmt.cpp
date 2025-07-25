#include "../ast/stmt.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/types.hpp"
#include "type.hpp"
#include "typeMaps.hpp"

void TypeChecker::visitStmt(Node::Stmt *stmt) { StmtAstLookup(stmt); }

void TypeChecker::visitExprStmt(Node::Stmt *stmt) {
  ExprStmt *expr_stmt = static_cast<ExprStmt *>(stmt);
  visitExpr(expr_stmt->expr);
  return_type = nullptr;
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
  if (isLspMode) {
    LSPIdentifierType type = LSPIdentifierType::Type;
    if (const_stmt->value->kind == NodeKind::ND_FN_STMT) {
      type = LSPIdentifierType::Function;
    } else if (const_stmt->value->kind == NodeKind::ND_STRUCT_STMT) {
      type = LSPIdentifierType::Struct;
    } else if (const_stmt->value->kind == NodeKind::ND_ENUM_STMT) {
      type = LSPIdentifierType::Enum;
    } else if (const_stmt->value->kind == NodeKind::ND_VAR_STMT) {
      type = LSPIdentifierType::Variable;
    }
    lsp_idents.push_back(LSPIdentifier{
        .underlying = new SymbolType("you gotta be joking, right", SymbolType::Signedness::SIGNED),
        .type = type,
        .ident = const_stmt->name,
        .scope = "",
        .isDefinition = true,
        .line = (size_t)const_stmt->line,
        .pos = (size_t)const_stmt->pos + 1,
        .fileID = (size_t)const_stmt->file_id,
    });
  }
  visitStmt(const_stmt->value);
}

void TypeChecker::visitFn(Node::Stmt *stmt) {
  FnStmt *fn_stmt = static_cast<FnStmt *>(stmt);
  context->enterScope(); // Enter function scope

  // Declare function in local table
  context->declareLocal(
      fn_stmt->name,
      fn_stmt->returnType); // Declare this first, so that recursion and stuff
                            // is possible later
  
  function_name = fn_stmt->name;

  // if the isTemplate bool is true declare T as a type that can be Used
  if (fn_stmt->isTemplate) {
    for (std::string t : fn_stmt->typenames) {
      context->declareLocal(t, new SymbolType(t));
      context->globalSymbols[t] = new SymbolType(t);
    }
  }

  // Check function parameters
  ParamsAndTypes params;
  for (std::pair<IdentExpr *, Node::Type *> &param : fn_stmt->params) {
    if (!param.second) {
      std::cerr << "Error: Parameter " << param.first << " has nullptr type!"
                << std::endl;
      continue;
    }
    params[param.first->name] = param.second;
    context->declareLocal(param.first->name, param.second);
  }

  // Add function to functionTable
  // check if the function is already declared
  if (context->functionTable.contains(fn_stmt->name)) {
    std::string msg = "Function '" + fn_stmt->name + "' already declared";
    handleError(fn_stmt->line, fn_stmt->pos - 2, msg, "", "Type Error", 
                fn_stmt->pos);
    // return;
  }
  context->functionTable.declare(fn_stmt->name, params, fn_stmt->returnType);

  visitStmt(fn_stmt->block);

  if (type_to_string(fn_stmt->returnType) != "void") {
    // Ensure return type is properly checked
    if (return_type == nullptr) {
      handleError(fn_stmt->line, fn_stmt->pos, "Return type is not defined", "",
                  "Type Error");
    }

    // Check return statement requirements
    if (!needsReturn) {
      handleError(fn_stmt->line, fn_stmt->pos,
                  "Function requires a return statement", "", "Type Error");
      return;
    }
  }

  // Ensure function return type matches expected type
  std::string expectedType = type_to_string(fn_stmt->returnType);
  std::string actualType = type_to_string(return_type.get());

  std::string msg = "Function '" + fn_stmt->name +
                    "' requeries a return type of '" +
                    type_to_string(fn_stmt->returnType) + "' but got '" +
                    type_to_string(return_type.get()) + "' instead.";
  bool check = checkTypeMatch(fn_stmt->returnType, return_type.get());
  if (!check) {
    handleError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  // Declare function in global scope
  context->declareGlobal(fn_stmt->name, fn_stmt->returnType);

  // Clear the template type from the local table
  if (fn_stmt->isTemplate) {
    for (std::string t : fn_stmt->typenames) {
      context->globalSymbols.erase(t);
      context->functionTable.erase(t);
    }
  }

  function_name = "";
  return_type = nullptr;
  context->exitScope();
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

  context->structTable.insert(
      {struct_stmt->name, {}}); // Declare blank and insert later

  // visit the fields Aka the variables
  for (std::pair<IdentExpr *, Node::Type *> &field : struct_stmt->fields) {
    context->structTable.addMember(struct_stmt->name, field.first->name,
                                   field.second);
    if (isLspMode) {
      lsp_idents.push_back(LSPIdentifier{
          .underlying = field.second,
          .type = LSPIdentifierType::StructMember,
          .ident = field.first->name,
          .scope = struct_stmt->name,
          .isDefinition = true,
          .line = (size_t)field.first->line,
          .pos = (size_t)field.first->pos - field.first->name.size(),
          .fileID = (size_t)field.first->file_id,
      });
    }
  }

  // handle fn stmts in the struct
  ParamsAndTypes params;
  for (Node::Stmt *stmt : struct_stmt->stmts) {
    context->enterScope();
    FnStmt *fn_stmt = static_cast<FnStmt *>(stmt);

    for (std::pair<IdentExpr *, Node::Type *> &param : fn_stmt->params) {
      // add the params to the local table
      params[param.first->name] = param.second;
      context->declareLocal(param.first->name, param.second);
    }

    // declare_struct_fn(map, {fn_stmt->name, fn_stmt->returnType}, params,
    //                   fn_stmt->line, fn_stmt->pos, struct_stmt->name);
    context->structTable.addFunction(struct_stmt->name, fn_stmt->name,
                                     fn_stmt->returnType, params);

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
                        "' requires a return stmt "
                        "but none was found";
      handleError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
      return;
    }

    std::string msg = "Function '" + fn_stmt->name +
                      "' requires a return type of '" +
                      type_to_string(fn_stmt->returnType) + "' but got '" +
                      type_to_string(return_type.get()) + "' instead.";
    bool check = checkTypeMatch(fn_stmt->returnType, return_type.get());
    if (!check) {
      handleError(fn_stmt->line, fn_stmt->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return;
    }

    // also add the function name to the global table and function table
    context->declareGlobal(fn_stmt->name, fn_stmt->returnType);

    return_type = nullptr;
    context->exitScope();
  }

  return_type = std::make_shared<SymbolType>(struct_stmt->name);
}

void TypeChecker::visitEnum(Node::Stmt *stmt) {
  EnumStmt *enum_stmt = static_cast<EnumStmt *>(stmt);
  SymbolType *type = new SymbolType("enum");

  // add the enum name to the local table and global table
  context->declareLocal(enum_stmt->name, static_cast<Node::Type *>(type));
  context->declareGlobal(enum_stmt->name, static_cast<Node::Type *>(type));
  // check if the enum table has the thing already declared
  if (context->enumTable.contains(enum_stmt->name)) {
    std::string msg = "Enum '" + enum_stmt->name + "' is already declared";
    handleError(enum_stmt->line, enum_stmt->pos, msg, "", "Type Error");
    return;
  }
  // delcare the enum in the enum table
  context->enumTable.declare(enum_stmt->name);

  if (enum_stmt->fields.empty()) {
    std::string msg =
        "Enum '" + enum_stmt->name + "' must have at least one field";
    handleError(enum_stmt->line, enum_stmt->pos, msg, "", "Type Error");
  }

  // visit the fields Aka the variables
  for (size_t i = 0; i < enum_stmt->fields.size(); i++) {
    context->enumTable.addMember(enum_stmt->name, enum_stmt->fields[i]->name,
                                 (long long)i);
    context->declareLocal(enum_stmt->fields[i]->name,
                                  static_cast<Node::Type *>(new SymbolType("int")));
    if (isLspMode) {
      lsp_idents.push_back(LSPIdentifier{
          .underlying = new SymbolType(enum_stmt->name),
          .type = LSPIdentifierType::EnumMember,
          .ident = enum_stmt->fields[i]->name,
          .scope = enum_stmt->name,
          .isDefinition = true,
          .line = (size_t)enum_stmt->fields[i]->line,
          .pos = (size_t)enum_stmt->fields[i]->pos - enum_stmt->fields[i]->name.size(),
          .fileID = (size_t)enum_stmt->file_id,
      });
    }
  }

  // set return type to the name of the enum
  return_type = std::make_shared<SymbolType>(enum_stmt->name);
}

void TypeChecker::visitIf(Node::Stmt *stmt) {
  IfStmt *if_stmt = static_cast<IfStmt *>(stmt);
  visitExpr(if_stmt->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "If condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(if_stmt->line, if_stmt->pos, msg, "", "Type Error");
  }
  visitStmt(if_stmt->thenStmt);
  if (if_stmt->elseStmt != nullptr) {
    visitStmt(if_stmt->elseStmt);
  }
}

void TypeChecker::visitVar(Node::Stmt *stmt) {
  VarStmt *var_stmt = static_cast<VarStmt *>(stmt);
  if (context->globalSymbols.contains(var_stmt->name) ||
      context->lookup(var_stmt->name)) {
    // If the variable is already declared in the global or local scope
    // throw an error
    std::string msg = "Variable '" + var_stmt->name + "' already declared";
    TypeChecker::handleError(var_stmt->line, var_stmt->pos, msg, "", "Type Error", (int)(var_stmt->pos + var_stmt->name.size()));
    return;
  }
  context->declareLocal(var_stmt->name, var_stmt->type);

  // check if the type is a struct or enum
  if (context->structTable.contains(type_to_string(var_stmt->type)) ||
      context->enumTable.contains(type_to_string(var_stmt->type))) {
    return_type = share(var_stmt->type);
  }
  if (isLspMode) {
    lsp_idents.push_back(LSPIdentifier {
      .underlying = var_stmt->type,
      .type = LSPIdentifierType::Variable,
      .ident = var_stmt->name,
      .scope = function_name,
      .isDefinition = true,
      .line = (unsigned long)var_stmt->line,
      .pos = (unsigned long)var_stmt->pos + 1, // i dont know why you have to add one. it just looked off
      .fileID = (unsigned long)var_stmt->file_id,
    });
  }

  // Now check if we have a template struct
  if (var_stmt->type->kind == ND_TEMPLATE_STRUCT_TYPE) {
    TemplateStructType *temp =
        static_cast<TemplateStructType *>(var_stmt->type);

    // auto res = map->struct_table.find(type_to_string(temp->name));
    auto res = context->structTable.contains(type_to_string(temp->name));
    if (!res) {
      std::string msg =
          "Template struct '" + type_to_string(temp->name) + "' is not defined";
      handleError(var_stmt->line, var_stmt->pos, msg, "", "Type Error");
    }

    // If we found the struct now lets go through the fields and find the value
    for (auto &field : context->structTable.at(type_to_string(temp->name))) {
      // declare the field in the local table
      context->declareLocal(field.first, field.second.first);
    }

    // Now we can set the return type to the underlying type
    return_type = share(temp->underlying);

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
      var_stmt->type = new ArrayType(array_type->underlying,
                                     (long long)array_expr->elements.size());
    } else if (var_stmt->expr->kind ==
               NodeKind::ND_ARRAY) { // auto filled arrays will always have 1
                                     // element (the one to autofill) so do not
                                     // error in that case
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
    array_expr->type = new ArrayType(array_type->underlying,
                                     (long long)array_expr->elements.size());
    return_type = share(array_type);
    visitExpr(
        var_stmt->expr); // Visit the array, and by extension, its elements
    return_type = share(array_type);
  } else if (var_stmt->type->kind == ND_POINTER_TYPE) {
    PointerType *ptr = static_cast<PointerType *>(var_stmt->type);
    if (var_stmt->expr->kind == NodeKind::ND_NULL) {
      return_type = share(ptr);
    } else {
      visitExpr(var_stmt->expr);
      return_type = share(ptr);
    }
  } else {
    visitExpr(var_stmt->expr);
  }

  // check if the variable type is the same as the expr type
  std::string msg = "Variable '" + var_stmt->name + "' requires type '" +
                    type_to_string(var_stmt->type) + "' but got '" +
                    type_to_string(return_type.get()) + "'";
  if (!checkTypeMatch(var_stmt->type, return_type.get())) {
    handleError(var_stmt->line, var_stmt->pos, msg, "", "Type Error");
  }

  return_type = nullptr;
}

void TypeChecker::visitPrint(Node::Stmt *stmt) {
  OutputStmt *print_stmt = static_cast<OutputStmt *>(stmt);
  visitExpr(print_stmt->fd);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Print requires the file descriptor to be of type 'int!' "
                      "but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(print_stmt->line, print_stmt->pos, msg, "", "Type Error");
  }

  // check if we are a println statement if so add a new line to the end
  if (print_stmt->isPrintln)
    print_stmt->args.push_back(new StringExpr(0, 0, "\"\\n\"", 0));

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

  if (return_type.get()->kind != ND_SYMBOL_TYPE) {
    if (static_cast<SymbolType *>(return_type.get())->name != "bool") {
      std::string msg = "For loop condition must be a 'bool' but got '" +
                        type_to_string(return_type.get()) + "'";
      handleError(for_stmt->line, for_stmt->pos, msg, "", "Type Error");
    }
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

  if (return_type.get()->kind != ND_SYMBOL_TYPE) {
    if (static_cast<SymbolType *>(return_type.get())->name != "bool") {
      std::string msg = "While loop condition must be a 'bool' but got '" +
                        type_to_string(return_type.get()) + "'";
      handleError(while_stmt->line, while_stmt->pos, msg, "", "Type Error");
    }
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
  // Convert the relative path of the import's name to be absolute
  std::filesystem::path import_path = std::filesystem::absolute(
      std::filesystem::path(import_stmt->name));
  if (importedFiles.contains(import_path.string())) {
    // If the import has already been imported, we do not need to do anything
    return;
  }
  // store the current file name
  std::string file_name = node.current_file;
  // If the path of the import is absolute, we must set the node.current_file to
  
  std::unordered_map<std::string, Node::Type *>::iterator res =
      context->globalSymbols.find(import_stmt->name);
  if (res != context->globalSymbols.end()) {
    std::string msg = "'" + import_stmt->name + "' has already been imported.";
    handleError(import_stmt->line, import_stmt->pos, msg, "", "Type Error");
    return;
  }
  
  // be the import path relative to the cwd
  if (std::filesystem::path(import_stmt->name).is_absolute()) {
    node.current_file =
        std::filesystem::path(import_stmt->name).relative_path().string();
  } else {
    node.current_file = import_stmt->name;
  }

  context->declareGlobal(import_stmt->name, return_type.get());
  std::swap(node.tks, import_stmt->tks); // C++ my beloved
  // type check the import
  visitStmt(import_stmt->stmt);
  
  return_type = nullptr;
  node.current_file = file_name; // reset the current file name
  std::swap(node.tks, import_stmt->tks);
  // Add the absolute path to the imported files
  importedFiles.insert(import_path.string());
}

void TypeChecker::visitMatch(Node::Stmt *stmt) {
  MatchStmt *match_stmt = static_cast<MatchStmt *>(stmt);
  visitExpr(match_stmt->coverExpr);

  for (std::pair<Node::Expr *, Node::Stmt *> &pair : match_stmt->cases) {
    Node::Expr *case_expr = pair.first;
    Node::Stmt *case_stmt = pair.second;

    // Visit the case expression to get its type
    visitExpr(case_expr);

    // Check if the case expression is of the same type as the cover expression
    if (!checkTypeMatch(return_type.get(), match_stmt->coverExpr->asmType)) {
      std::string msg = "Case expression type '" +
                        type_to_string(return_type.get()) +
                        "' does not match cover expression type '" +
                        type_to_string(match_stmt->coverExpr->asmType) + "'";
      handleError(match_stmt->line, match_stmt->pos, msg, "", "Type Error");
    }

    // Visit the case statement
    visitStmt(case_stmt);
  }

  // Check if there is a default case
  if (match_stmt->defaultCase != nullptr) {
    // Visit the default case statement
    visitStmt(match_stmt->defaultCase);
  } else {
    std::string msg = "Match statement does not have a default case.";
    handleError(match_stmt->line, match_stmt->pos, msg, "",
                "Type Error");
  }

  // Set the return type to void since match statements do not return a value
  return_type = nullptr;
}

void TypeChecker::visitLink(Node::Stmt *stmt) {
  (void)stmt; // Mark it as unused
  // nothing to do here
}

void TypeChecker::visitExtern(Node::Stmt *stmt) {
  (void)stmt; // mark it as unused
  // nothing to do here
}

void TypeChecker::visitBreak(Node::Stmt *stmt) {
  (void)stmt; // mark it as unused
  // nothing to do here
}

void TypeChecker::visitContinue(Node::Stmt *stmt) {
  (void)stmt; // mark it as unused
  // nothing to do here
}

//              varName,     bytes
// @input( (char* | []char),  int)
void TypeChecker::visitInput(Node::Stmt *stmt) {
  InputStmt *input_stmt = static_cast<InputStmt *>(stmt);

  visitExpr(input_stmt->fd);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Input system call fd must be a 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  visitExpr(input_stmt->bufferOut); // str, char*, char[]
  bool isStrType = return_type.get()->kind == ND_SYMBOL_TYPE &&
                   type_to_string(return_type.get()) == "str";
  bool isCharPtrType =
      return_type.get()->kind == ND_POINTER_TYPE &&
      static_cast<SymbolType *>(
          static_cast<PointerType *>(return_type.get())->underlying)
              ->name == "char";
  bool isCharArrayType =
      return_type.get()->kind == ND_ARRAY_TYPE &&
      static_cast<SymbolType *>(
          static_cast<ArrayType *>(return_type.get())->underlying)
              ->name == "char";
  if (!isStrType && !isCharPtrType && !isCharArrayType) {
    std::string msg = "Input variable name must be a 'string', 'char*' or "
                      "'char[]' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  visitExpr(input_stmt->maxBytes);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Input max bytes must be a 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(input_stmt->line, input_stmt->pos, msg, "", "Type Error");
  }

  return_type = nullptr;
}

void TypeChecker::visitClose(Node::Stmt *stmt) {
  CloseStmt *close_stmt = static_cast<CloseStmt *>(stmt);
  visitExpr(close_stmt->fd);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Close system call fd must be a 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(close_stmt->line, close_stmt->pos, msg, "", "Type Error");
  }
  return_type = nullptr;
}
