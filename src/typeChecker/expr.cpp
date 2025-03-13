#include "../ast/ast.hpp"
#include "../ast/types.hpp"
#include "../helper/math/math.hpp"
#include "typeMaps.hpp"
#include "type.hpp"

#include <algorithm>
#include <optional>
#include <memory>
#include <limits>
#include <string>

void TypeChecker::visitExpr(Node::Expr *expr) {
  ExprAstLookup(expr);
}

void TypeChecker::visitTemplateCall(Node::Expr *expr) {
  TemplateCallExpr *template_call = static_cast<TemplateCallExpr *>(expr);

  std::string msg = "Template function calls are not supported yet";
  handleError(template_call->line, template_call->pos, msg, "", "Type Error");
}

void TypeChecker::visitExternalCall(Node::Expr *expr) {
  // There's not really a whole lot to typecheck here , lol
  return_type = std::make_shared<SymbolType>(
      "unknown"); // Unknown type, imagine that this is cast to like int or
                  // whatever
  expr->asmType = new SymbolType("unknown");
};

void TypeChecker::visitInt(Node::Expr *expr) {
  IntExpr *integer = static_cast<IntExpr *>(expr);

  // check if the integer is within the range of an i64 int
  if (integer->value > std::numeric_limits<signed long long int>::max() ||
      integer->value < std::numeric_limits<signed long long int>::min()) {
    std::string msg = "Integer '" + std::to_string(integer->value) +
                      "' is out of range for an 'int' which is 64 bits";
    handleError(integer->line, integer->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
  expr->asmType = new SymbolType("int");
}

void TypeChecker::visitFloat(Node::Expr *expr) {
  FloatExpr *floating = static_cast<FloatExpr *>(expr);

  // check if the float is within the range of an f32 float
  if (floating->value > std::numeric_limits<float>::max() ||
      floating->value < std::numeric_limits<float>::min()) {
    std::string msg = "Float '" + std::to_string(floating->value) +
                      "' is out of range for a 'float' which is 32 bits";
    handleError(floating->line, floating->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("float");
  expr->asmType = new SymbolType("float");
}

void TypeChecker::visitString(Node::Expr *expr) {
  return_type = std::make_shared<SymbolType>("str");
  expr->asmType = new SymbolType("str");
}

void TypeChecker::visitChar(Node::Expr *expr) {
  return_type = std::make_shared<SymbolType>("char");
  expr->asmType = new SymbolType("char");
}

void TypeChecker::visitAddress(Node::Expr *expr) {
  AddressExpr *address = static_cast<AddressExpr *>(expr);
  visitExpr(address->right);
  return_type = std::make_shared<PointerType>(return_type.get());
  expr->asmType = new PointerType(createDuplicate(address->right->asmType));
}

void TypeChecker::visitDereference(Node::Expr *expr) {
  DereferenceExpr *dereference = static_cast<DereferenceExpr *>(expr);
  // look though the local and global symbol table to find the type of the
  visitExpr(dereference->left);

  // check if the left side is a pointer
  auto it = checkReturnType(dereference->left, "unknown");  
  if (type_to_string(it.get()).find("*") == std::string::npos) {
    std::string msg = "Dereference operation requires the left hand side to be a pointer but got '" + type_to_string(it.get()) + "'";
    handleError(dereference->line, dereference->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // remove the '*' from the type
  std::string ptr = it.get()->name.replace(it.get()->name.find("*"), 1, "");
  return_type = std::make_shared<SymbolType>(ptr);
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitIdent(Node::Expr *expr) {
  IdentExpr *ident = static_cast<IdentExpr *>(expr);
  Node::Type *res = nullptr;

  for (auto it : context->localScopes) {
    if (it.find(ident->name) != it.end()) {
      res = it[ident->name];
      ident->asmType = createDuplicate(res);
    }
  }

  // check if we found something in the local symbol table if not return error of 'did you mean'
  if (res == nullptr) {
    for (auto it : context->localScopes) {
      for (auto pair : it) {
        context->stackKeys.push_back(pair.first);
      }
    }
    std::optional<std::string> closest = string_distance(context->stackKeys, ident->name, 3);
    std::string msg = (closest.has_value()) ? "Undefined variable '" + ident->name + "' did you mean '" + closest.value() + "'?" 
                                            : "Undefined variable '" + ident->name + "'";
    handleError(ident->line, ident->pos, msg, "", "Type Error");
    res = new SymbolType("unknown"); // return unknown type
  }

  // update the ast-node (IdentExpr) to hold the type of the identifier as a
  // property
  ident->type = res;
  if (res->kind == ND_ARRAY_TYPE) {
    ArrayType *at = static_cast<ArrayType *>(res);
    return_type = std::make_shared<ArrayType>(at->underlying, at->constSize);
  } else {
    return_type = std::make_shared<SymbolType>(type_to_string(res));
  }
  // TODO: Update this to the new tc system
  // if (isLspMode) {
  //   // Check if enum type
  //   if (map->enum_table.find(ident->name) != map->enum_table.end()) {
  //     lsp_idents.push_back(LSPIdentifier{res, LSPIdentifierType::Enum, ident->name, ident->line, ident->pos});
  //     return;
  //   }
  //   // check if struct type
  //   if (map->struct_table.find(ident->name) != map->struct_table.end()) {
  //     lsp_idents.push_back(LSPIdentifier{res, LSPIdentifierType::Struct, ident->name, ident->line, ident->pos});
  //     return;
  //   }
  //   // check if function
  //   // loop over functions
  //   for (auto fn : map->function_table) {
  //     if (fn.first.first == ident->name) {
  //       lsp_idents.push_back(LSPIdentifier{res, LSPIdentifierType::Function, ident->name, ident->line, ident->pos});
  //       return;
  //     } // my pc is about to explode of battery lol fun lol
  //   }
  //   lsp_idents.push_back(LSPIdentifier{res, LSPIdentifierType::Variable, ident->name, ident->line, ident->pos});
  // }
}

void TypeChecker::visitBinary(Node::Expr *expr) {
  BinaryExpr *binary = static_cast<BinaryExpr *>(expr);

  visitExpr(binary->lhs);
  std::shared_ptr<SymbolType> lhs = checkReturnType(binary->lhs, "unknown");
  visitExpr(binary->rhs);
  std::shared_ptr<SymbolType> rhs = checkReturnType(binary->rhs, "unknown");

  std::string msg = "Binary operation '" + binary->op +
                    "' requires the type to be an 'int' or 'float' but got '" +
                    type_to_string(lhs.get()) + "' and '" +
                    type_to_string(rhs.get()) + "'";
  
  if (!checkTypeMatch(lhs, rhs, binary->op, binary->line, binary->pos, msg)) {
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  if (mathOps.find(binary->op) != mathOps.end()) {
    return_type = lhs;
    expr->asmType = new SymbolType(lhs.get()->name); // This, for some reason, is needed, otherwise the type is not set
  } else if (boolOps.find(binary->op) != boolOps.end()) {
    return_type = std::make_shared<SymbolType>("bool");
    expr->asmType = new SymbolType("bool");
  } else {
    std::string msg = "Unsupported binary operator: " + binary->op;
    handleError(binary->line, binary->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
  }
}

void TypeChecker::visitUnary(Node::Expr *expr) {
  UnaryExpr *unary = static_cast<UnaryExpr *>(expr);

  if (unary->op == "-") {
    // update the bool on the value to be negative
    IntExpr *integer = static_cast<IntExpr *>(unary->expr);
    integer->isNegative = true;
    visitExpr(integer);

    if (!isIntBasedType(return_type.get())) {
      std::string msg = "Unary operation '" + unary->op + "' requires the type to be an 'int' but got '" + type_to_string(return_type.get()) + "'";
      handleError(unary->line, unary->pos, msg, "", "Type Error");
    }

    return;
  }

  visitExpr(unary->expr);

  return_type = checkReturnType(expr, "unknown");
  expr->asmType = createDuplicate(return_type.get());

  if (unary->op == "!" && type_to_string(return_type.get()) != "bool") {
    std::string msg = "Unary operation '" + unary->op + "' requires the type to be a 'bool' but got '" + type_to_string(return_type.get()) + "'";
    handleError(unary->line, unary->pos, msg, "", "Type Error");
  }

  // check for ++ and --
  if (unary->op == "++" || unary->op == "--") {
    if (type_to_string(return_type.get()) != "int") {
      std::string msg = "Unary operation '" + unary->op + "' requires the type to be an 'int' but got '" + type_to_string(return_type.get()) + "'";
      handleError(unary->line, unary->pos, msg, "", "Type Error");
    }
  }
}

void TypeChecker::visitGrouping(Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);
  visitExpr(grouping->expr);
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitBool(Node::Expr *expr) {
  return_type = std::make_shared<SymbolType>("bool");
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitTernary(Node::Expr *expr) {
  TernaryExpr *ternary = static_cast<TernaryExpr *>(expr);
  visitExpr(ternary->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "Ternary condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(ternary->line, ternary->pos, msg, "", "Type Error");
  }

  visitExpr(ternary->lhs);
  std::shared_ptr<Node::Type> lhs = return_type;
  visitExpr(ternary->rhs);
  std::shared_ptr<Node::Type> rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
    std::string msg =
        "Ternary operation requires both sides to be the same type";
    handleError(ternary->line, ternary->pos, msg, "", "Type Error");
  }

  return_type = lhs;
  expr->asmType = lhs.get();
}

void TypeChecker::visitAssign(Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
  visitExpr(assign->assignee);
  std::shared_ptr<Node::Type> lhs = return_type;
  visitExpr(assign->rhs);
  std::shared_ptr<Node::Type> rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  std::string msg = "Assignment requires both sides to be the same type, but got '" + type_to_string(lhs.get()) + "' and '" + type_to_string(rhs.get()) + "'";
  checkTypeMatch(std::make_shared<SymbolType>(type_to_string(lhs.get())),
                 std::make_shared<SymbolType>(type_to_string(rhs.get())), "=",
                 assign->line, assign->pos, msg);

  return_type = lhs;
  expr->asmType = lhs.get();
}

void TypeChecker::visitCall(Node::Expr *expr) {
  CallExpr *call = static_cast<CallExpr *>(expr);
  Node::Expr *name = call->callee;

  // check if the callee is an identifier
  if (name->kind != ND_IDENT && name->kind != ND_MEMBER) {
    std::string msg = "Function call requires the callee to be an identifier";
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // check if the callee is a function
  std::pair<std::string, Node::Type *> fnName;
  if (name->kind == ND_IDENT) {
    IdentExpr *ident = static_cast<IdentExpr *>(name);
    fnName = {ident->name, nullptr};
  } else {
    MemberExpr *member = static_cast<MemberExpr *>(name);
    fnName = {static_cast<IdentExpr *>(member->lhs)->name, nullptr};
  }

  // check if the function is in the function table
  if (context->functionTable.find({fnName.first, fnName.second}) != context->functionTable.end()) {
    std::string msg = "Function '" + fnName.first + "' not declared";
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // get the function parameters
  auto it = context->functionTable.getParams(fnName.first, fnName.second);
  std::unordered_map<std::string, Node::Type *> fnParams;
  for (auto param : it.params) fnParams[param.first] = param.second;

  if (!validateArgumentCount(call, name, fnParams)) return;

  if (!validateArgumentTypes(call, name, fnParams)) return;

  // Set return type to the return of the fn
  if (fnName.second->kind == ND_POINTER_TYPE) {
    return_type = std::make_shared<PointerType>(
        static_cast<PointerType *>(fnName.second)->underlying);
  } else if (fnName.second->kind == ND_ARRAY_TYPE) {
    ArrayType *at = static_cast<ArrayType *>(fnName.second);
    return_type = std::make_shared<ArrayType>(at->underlying, at->constSize);
  } else {
    return_type = std::make_shared<SymbolType>(type_to_string(fnName.second));
  }
  expr->asmType = createDuplicate(return_type.get());
  // add an ident // ok bye its about to shutdown now grab the charger! im too far :(((((  RUNNNNNNNNNN 
  if (isLspMode) {
    lsp_idents.push_back(LSPIdentifier{fnName.second, LSPIdentifierType::Function, fnName.first, static_cast<IdentExpr *>(call->callee)->line, static_cast<IdentExpr *>(call->callee)->pos});
  }
}

void TypeChecker::visitMember(Node::Expr *expr) {
  MemberExpr *member = static_cast<MemberExpr *>(expr);

  // Verify that the lhs is a struct or enum
  visitExpr(member->lhs);
  std::string lhsType = type_to_string(return_type.get());
  std::string name = static_cast<IdentExpr *>(member->rhs)->name;

  // Determine if lhs is a struct or enum
  std::string typeKind = determineTypeKind(lhsType);

  // check if the lhs is a struct or enum
  if (context->structTable.find(lhsType) == context->structTable.end() && context->enumTable.find(name) == context->enumTable.end()) {
    std::string msg = "Member access requires the left hand side to be a struct or enum but got '" + lhsType + "'";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // check if the lhs is a struct
  if (typeKind == "struct") {
    processStructMember(member, name, lhsType);
  } else if (typeKind == "enum") {
    processEnumMember(member, name);
  } else {
    std::string msg = "Member access requires the left hand side to be a struct or enum but got '" + lhsType + "' for '" + name + "'";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
  }
}

void TypeChecker::visitArray(Node::Expr *expr) {
  ArrayExpr *array = static_cast<ArrayExpr *>(expr);
  ArrayType *at = static_cast<ArrayType *>(createDuplicate(return_type.get()));

  
  // check if the array is empty
  if (array->elements.empty()) {
    std::string msg = "Array must have at least one element!";
    handleError(array->line, array->pos, msg, "", "Type Error");
    return_type = std::make_shared<ArrayType>(new SymbolType("unknown"), -1);
    expr->asmType = createDuplicate(return_type.get());
    return;
  }

  if (at->constSize != array->elements.size()) {
    std::string msg = "Array requires " +
                      std::to_string(at->constSize) + " elements but got " +
                      std::to_string(array->elements.size());
    handleError(array->line, array->pos, msg, "", "Type Error");
  }

  for (Node::Expr *elem : array->elements) {
    visitExpr(elem);
    if (return_type == nullptr) {
      return_type = std::make_shared<SymbolType>("unknown");
      expr->asmType = createDuplicate(return_type.get());
      return;
    }
    if (type_to_string(return_type.get()) != type_to_string(at->underlying)) {
      // It might be an int comparison that's really weird
      if (isIntBasedType(return_type.get()) && isIntBasedType(at->underlying)) {
        continue; // Don't error- this is okay.
      }
      std::string msg = "Array requires all elements to be of type '" +
                        type_to_string(at->underlying) + "' but got '" +
                        type_to_string(return_type.get()) + "'";
      handleError(array->line, array->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      expr->asmType = new SymbolType("unknown");
      return;
    }
  }

  return_type = std::make_shared<ArrayType>(at);
  expr->asmType = at;

  // clear the array table
  array->elements.clear();
}

void TypeChecker::visitArrayType(Node::Type *type) {
  ArrayType *array = static_cast<ArrayType *>(type);
  return_type =
      std::make_shared<ArrayType>(array->underlying, array->constSize);
}

void TypeChecker::visitIndex(Node::Expr *expr) {
  IndexExpr *index = static_cast<IndexExpr *>(expr);
  auto lhs = index->lhs;
  auto rhs = index->rhs;

  visitExpr(lhs);
  std::shared_ptr<Node::Type> lhsType = return_type;

  visitExpr(rhs);
  std::shared_ptr<Node::Type> rhsType = return_type;

  // search for '[]' in the lhs type and 'int' in the rhs type
  auto lhsStr = type_to_string(lhsType.get());
  auto rhsStr = type_to_string(rhsType.get());

  if (lhsStr.find("[]") == std::string::npos) {
    std::string msg = "Indexing requires the left hand side to be an array but got '" + lhsStr + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  if (rhsStr != "int") {
    std::string msg = "Indexing requires the right hand side to be an 'int' but got '" + rhsStr + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // set the return type to the underlying type of the array
  return_type = std::make_shared<SymbolType>(lhsStr.substr(2, lhsStr.size() - 2)); // remove the '[]' from the type
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitArrayAutoFill(Node::Expr *expr) {
  ArrayAutoFill *array = static_cast<ArrayAutoFill *>(expr);

  ArrayType *arrayType = static_cast<ArrayType *>(createDuplicate(return_type.get()));
  array->fillType = createDuplicate(arrayType->underlying);
  array->fillCount = arrayType->constSize;
  return_type = std::make_shared<ArrayType>(array->fillType, array->fillCount);
  expr->asmType = createDuplicate(return_type.get());
  // assign the fill type to the array type
  if (array->fillType->kind == ND_ARRAY_TYPE) {
    ArrayType *at = static_cast<ArrayType *>(array->fillType);
    if (at->constSize != -1) {
      std::string msg = "Auto-filled arrays cannot have a pre-determined array to copy.";
      handleError(array->line, array->pos, msg, "", "Type Error");
    }
  }
}

void TypeChecker::visitCast(Node::Expr *expr) {
  // Cast the generic expression to a CastExpr
  CastExpr *cast = static_cast<CastExpr *>(expr);
  // Visit the inside (update it's inner asmType)
  visitExpr(cast->castee);
  expr->asmType = cast->castee_type;
  return_type = std::make_shared<SymbolType>(type_to_string(cast->castee_type));
}

void TypeChecker::visitStructExpr(Node::Expr *expr) {
  StructExpr *struct_expr = static_cast<StructExpr *>(expr);

  std::string struct_name = "";
  if (return_type.get() == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
  } else if (return_type.get()->kind == ND_ARRAY_TYPE) {
    // The struct name will actually have to be the underlying, because
    // obviously you cannot define a struct's name with `[]`.
    struct_name =
        type_to_string(static_cast<ArrayType *>(return_type.get())->underlying);
  } else {
    struct_name = type_to_string(return_type.get());
  }

  // find the struct in the struct table (no it does not work) I know why
  if (context->structTable.find(struct_name) == context->structTable.end()) {
    std::string msg = "Struct '" + struct_name + "' is not defined in the scope.";
    handleError(struct_expr->line, struct_expr->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    struct_expr->asmType = new SymbolType("unknown");
    return;
  }

  // check if the number of elements in the struct is the same as the number of
  // elements in the struct expression
  int struct_size = 0;
  for (auto it : context->structTable[struct_name]) struct_size++;

  // We do not need ti check the struct size if the struct expression is empty
  if (struct_expr->values.empty()) {
    return_type = std::make_shared<SymbolType>(struct_name);
    struct_expr->asmType = new SymbolType(struct_name);
    return;
  }

  if ((size_t)struct_size != struct_expr->values.size()) {
    std::string msg = "Struct '" + struct_name + "' requires " +
                      std::to_string(struct_size) + " elements but got " +
                      std::to_string(struct_expr->values.size());
    handleError(struct_expr->line, struct_expr->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    struct_expr->asmType = new SymbolType("unknown");
    return;
  }

  // Now compare the types of the struct elements with the types of the struct
  // expression elements
  for (std::pair<std::string, Node::Expr *> elem : struct_expr->values) {
    // find the type of the struct element;
    std::string elem_type = "";
    // set elem_type to the type of the struct element, if it exists
    for (const auto& member :
         context->structTable[struct_name]) {
      if (member.first == elem.first) {
        elem_type = type_to_string(member.second.first);
        break;
      }
    }

    if (elem_type == "") {
      std::string msg = "Named member '" + elem.first +
                        "' not found in struct '" + struct_name + "'";
      // did you mean?
      std::vector<std::string> known;
      for (const auto &member : context->structTable[struct_name]) {
        known.push_back(member.first);
      }
      std::optional<std::string> closest =
          string_distance(known, elem.first, 3);
      if (closest.has_value()) {
        handleError(struct_expr->line, struct_expr->pos, msg,
                    "Did you mean '" + closest.value() + "'?", "Type Error");
      } else {
        handleError(struct_expr->line, struct_expr->pos, msg, "", "Type Error");
      }
      return_type = std::make_shared<SymbolType>("unknown");
      struct_expr->asmType = new SymbolType("unknown");
      return;
    }
    // find if the type of the struct expression element is a nested struct of a
    // bigger one
    if (elem.second->kind == ND_STRUCT) {
      return_type = std::make_shared<SymbolType>(elem_type);
      struct_expr->asmType = new SymbolType(elem_type);
    }

    visitExpr(elem.second);
    // check for enums
    if (elem.second->kind == ND_MEMBER) {
      MemberExpr *member = static_cast<MemberExpr *>(elem.second);
      if (member->lhs->kind == ND_IDENT) {
        IdentExpr *ident = static_cast<IdentExpr *>(member->lhs);
        if (context->enumTable.find(ident->name) != context->enumTable.end()) {
          processEnumMember(member, ident->name); // Will automatically set return_type
          member->asmType = createDuplicate(return_type.get()); // maybe this works idrk lmao
        }
      }
    }
    std::string expected = type_to_string(elem.second->asmType);
    if (checkReturnType(elem.second, expected) == nullptr) {
      std::string msg = "Struct element '" + elem.first + "' requires type '" +
                        elem_type + "' but got '" + expected + "'";
      handleError(struct_expr->line, struct_expr->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return;
    }
  }
  // set the return type to the struct type
  return_type = std::make_shared<SymbolType>(struct_name);
  expr->asmType = new SymbolType(struct_name);
}

void TypeChecker::visitAllocMemory(Node::Expr *expr) {
  AllocMemoryExpr *alloc = static_cast<AllocMemoryExpr *>(expr);
  
  visitExpr(alloc->bytesToAlloc);
  if (type_to_string(return_type.get()) != "int") {
    std::string msg = "Alloc requires the number of bytes to be of type 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(alloc->line, alloc->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<PointerType>(new SymbolType("void"));
  // asmtype is a constant void* and already handled in ast
}

void TypeChecker::visitFreeMemory(Node::Expr *expr) {
  FreeMemoryExpr *freeMemory = static_cast<FreeMemoryExpr *>(expr);
  visitExpr(freeMemory->whatToFree);
  if (type_to_string(return_type.get())[0] != '*') {
    std::string msg = "Freeing memory requires the memory to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(freeMemory->line, freeMemory->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(freeMemory->bytesToFree);
  if (type_to_string(return_type.get()) != "int") {
    std::string msg = "Freeing memory requires the number of bytes to be of type 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(freeMemory->line, freeMemory->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
  // asmtype, once again, already handled
}

void TypeChecker::visitSizeof(Node::Expr *expr) {
  SizeOfExpr *sizeOf = static_cast<SizeOfExpr *>(expr);
  visitExpr(sizeOf->whatToSizeOf);
  return_type = std::make_shared<SymbolType>("int");
  // asmtype is a constant int and already handled
}

void TypeChecker::visitMemcpyMemory(Node::Expr *expr) {
  MemcpyExpr *memcpy = static_cast<MemcpyExpr *>(expr);
  visitExpr(memcpy->dest);
  if (type_to_string(return_type.get())[0] != '*') {
    std::string msg = "Memcpy requires the destination to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(memcpy->line, memcpy->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(memcpy->src);
  if (type_to_string(return_type.get())[0] != '*') {
    std::string msg = "Memcpy requires the source to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(memcpy->line, memcpy->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(memcpy->bytes);
  if (type_to_string(return_type.get()) != "int") {
    std::string msg = "Memcpy requires the number of bytes to be of type 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(memcpy->line, memcpy->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
  // asmtype, once again, already handled
}

void TypeChecker::visitOpen(Node::Expr *expr) {
  OpenExpr *open = static_cast<OpenExpr *>(expr);
  visitExpr(open->filename);
  if (type_to_string(return_type.get()) != "str"
      && type_to_string(return_type.get()) != "*char"
      && type_to_string(return_type.get()) != "[]char") {
    std::string msg = "Open expression requires the filepath to be of type 'str' or its derivatives but got '" + type_to_string(return_type.get()) + "'";
    handleError(open->line, open->pos, msg, "", "Type Error");
  }

  // There are 3 bools that can be passed to open, but they are all optional
  if (open->canRead != nullptr) {
    visitExpr(open->canRead);
    if (type_to_string(return_type.get()) != "bool") {
      std::string msg = "Open expression requires the canRead parameter to be of type 'bool' but got '" + type_to_string(return_type.get()) + "'";
      handleError(open->line, open->pos, msg, "", "Type Error");
    }
  }

  if (open->canWrite != nullptr) {
    visitExpr(open->canWrite);
    if (type_to_string(return_type.get()) != "bool") {
      std::string msg = "Open expression requires the canWrite parameter to be of type 'bool' but got '" + type_to_string(return_type.get()) + "'";
      handleError(open->line, open->pos, msg, "", "Type Error");
    }
  }

  return_type = std::make_shared<SymbolType>("int");
  // asmtype is a constant int and already handled
}