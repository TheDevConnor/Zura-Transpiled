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
#include <cstdlib>

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
  if (integer->value < 0) {
    // It is signed
    return_type = std::make_shared<SymbolType>("int", SymbolType::Signedness::SIGNED);
    expr->asmType = new SymbolType("$", SymbolType::Signedness::SIGNED);
  } else {
    // It is an unsigned int
    return_type = std::make_shared<SymbolType>("int", SymbolType::Signedness::UNSIGNED);
    expr->asmType = new SymbolType("$", SymbolType::Signedness::UNSIGNED);
  }
}

void TypeChecker::visitFloat(Node::Expr *expr) {
  FloatExpr *floating = static_cast<FloatExpr *>(expr);
  long double value = std::stold(floating->value); // When comparing values like this, we want as much precision as possible.
  // check if the float is within the range of an f32 float
  if (value > std::numeric_limits<float>::max() ||
      value < std::numeric_limits<float>::min()) {
    // Check if it's out of range for a DOUBLE
    if (value > std::numeric_limits<double>::max() ||
        value < std::numeric_limits<double>::min()) {
      std::string msg = "Floating point number '" + floating->value.substr(0, 10) + // We don't need more than 10 bits of precision in the output
                                                                    // becuase you probably already know what the float in question is
                        " ...' is out of range for a 'double' which is 64 bits";
      handleError(floating->line, floating->pos, msg, "", "Type Error");
    } else {
      // signedness does not affect doubles (always have a sign bit)
      // IEEE 754 my beloved
      return_type = std::make_shared<SymbolType>("double");
      expr->asmType = new SymbolType("double");
      return;
    }
  }
  // TODO: Implement bigger flaot types (double, long double)
  return_type = std::make_shared<SymbolType>("float");
  expr->asmType = new SymbolType("float");
}

void TypeChecker::visitString(Node::Expr *expr) { // Although this returns a str, this can be casted to a char* or char[] if needed
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
  if (return_type.get()->kind != ND_POINTER_TYPE) {
    std::string msg = "Dereference operation requires the left hand side to be a pointer but got '" + type_to_string(return_type.get()) + "'";
    handleError(dereference->line, dereference->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  return_type = share(static_cast<PointerType *>(return_type.get())->underlying);
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitIdent(Node::Expr *expr) {
  IdentExpr *ident = static_cast<IdentExpr *>(expr);
  Node::Type *res = nullptr;

  for (auto it : context->localScopes) {
    if (it.find(ident->name) != it.end()) {
      res = it[ident->name];
      ident->asmType = createDuplicate(res);
      break;
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
  ident->asmType = res;
  return_type = share(res);
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
  Node::Type *lhsType = createDuplicate(return_type.get());
  visitExpr(binary->rhs);
  Node::Type *rhsType = createDuplicate(return_type.get());
  std::string msg = "Binary operation '" + binary->op +
                    "' requires the type to be an 'int' or 'float' but got '" +
                    type_to_string(lhsType) + "' and '" +
                    type_to_string(rhsType) + "'";
  
  if (!checkTypeMatch(lhsType, rhsType) && !(isIntBasedType(lhsType) && isIntBasedType(rhsType))) {
    // make an error
    handleError(binary->line, binary->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  if (mathOps.find(binary->op) != mathOps.end()) {
    // Check additionally if this was a multiplication or division on signed ints
    if (binary->op == "*" || binary->op == "/") {
      if (lhsType->kind == ND_SYMBOL_TYPE && rhsType->kind == ND_SYMBOL_TYPE) {
        if (static_cast<SymbolType *>(lhsType)->signedness == SymbolType::Signedness::SIGNED &&
            static_cast<SymbolType *>(rhsType)->signedness == SymbolType::Signedness::SIGNED) {
          expr->asmType = createDuplicate(lhsType);
          static_cast<SymbolType *>(expr->asmType)->signedness = SymbolType::Signedness::UNSIGNED; // ignore this beaut
          return_type = share(expr->asmType);
          return;
        }
      }
    }
    return_type = share(lhsType);
    expr->asmType = createDuplicate(lhsType);
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
  
  visitExpr(unary->expr);

  if (unary->op == "!" && type_to_string(return_type.get()) != "bool") {
    std::string msg = "Unary operation '" + unary->op + "' requires the type to be a 'bool' but got '" + type_to_string(return_type.get()) + "'";
    handleError(unary->line, unary->pos, msg, "", "Type Error");
  }

  // check for ++ and --
  if (unary->op == "++" || unary->op == "--") {
    if (!isIntBasedType(return_type.get())) {
      std::string msg = "Unary operation '" + unary->op + "' requires the type to be an 'int' but got '" + type_to_string(return_type.get()) + "'";
      handleError(unary->line, unary->pos, msg, "", "Type Error");
    }
  }
}

void TypeChecker::visitGrouping(Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);
  visitExpr(grouping->expr);
  // no need to set the return type here
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
  Node::Type *lhs = createDuplicate(return_type.get());
  visitExpr(ternary->rhs);
  Node::Type *rhs = return_type.get(); // Technically, since we are not changing anything in here, we can be fine with not creating a duplicate

  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  if (!checkTypeMatch(lhs, rhs)) {
    std::string msg =
        "Ternary operation requires both sides to be the same type";
    handleError(ternary->line, ternary->pos, msg, "", "Type Error");
  }

  return_type = share(lhs);
  expr->asmType = lhs;
}

void TypeChecker::visitAssign(Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
  visitExpr(assign->assignee);
  Node::Type *lhs = createDuplicate(return_type.get());
  visitExpr(assign->rhs);
  Node::Type *rhs = return_type.get(); // This doesn't technically have to be duplicated since we are not changing anything in here

  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  std::string msg = "Assignment requires both sides to be the same type, but got '" + type_to_string(lhs) + "' and '" + type_to_string(rhs) + "'";
  if (!checkTypeMatch(lhs, rhs)) {
    handleError(assign->line, assign->pos, msg, "", "Type Error");
  }

  return_type = share(lhs);
  expr->asmType = lhs; // this could be either because they are both the same
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

  // check if the function is in the function table
  if (context->functionTable.find({fnName.first, fnName.second}) != context->functionTable.end()) {
    std::string msg = "Function '" + fnName.first + "' not declared";
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // // get the function parameters
  // std::unordered_map<std::string, Node::Type *> fnParams;
  // if (context->functionTable.find({fnName.first, fnName.second}) != context->functionTable.end()) {
  //   fnParams = context->functionTable.at({fnName.first, fnName.second}).params;
  // }

  // if (!validateArgumentCount(call, name, fnParams)) return;

  // if (!validateArgumentTypes(call, name, fnParams)) return;

  // set the return type of the call to the return type of the function
  for (auto it : context->functionTable) return_type = share(it.first.type);
  expr->asmType = createDuplicate(return_type.get());

  // add an ident for the lsp
  if (isLspMode) {
    lsp_idents.push_back(LSPIdentifier{fnName.second, LSPIdentifierType::Function, fnName.first, (size_t)static_cast<IdentExpr *>(call->callee)->line, (size_t)static_cast<IdentExpr *>(call->callee)->pos});
  }
}

void TypeChecker::visitMember(Node::Expr *expr) {
  MemberExpr *member = static_cast<MemberExpr *>(expr);

  std::string rhs = static_cast<IdentExpr*>(member->rhs)->name;
  std::string note = "";

  visitExpr(member->lhs);
  std::string type = type_to_string(return_type.get());
  if (type.at(0) == '*') {
    type = type.substr(1);
  }

  // if the lhs is not a struct or enum, return an error
  if (determineTypeKind(type) != "struct" && determineTypeKind(type) != "enum") {
    std::string msg = "Member access requires the left hand side to be a struct or enum but got '" + type + "'";
    if (type.at(0) == '*') note = "Try dereferencing the pointer first";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // process the member access
  if (determineTypeKind(type) == "struct") {
    processStructMember(member, rhs, type);
  } else if (determineTypeKind(type) == "enum") {
    processEnumMember(member, type);
  } else {
    std::string msg = "Member access requires the left hand side to be a struct or enum but got '" + type + "' for '" + rhs + "'";
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

  if (at->constSize != (long long)array->elements.size()) {
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
    if (!checkTypeMatch(at->underlying, return_type.get())) {
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
}

void TypeChecker::visitArrayType(Node::Type *type) {
  return_type = share(type); // Easy lol
}

void TypeChecker::visitIndex(Node::Expr *expr) {
  IndexExpr *index = static_cast<IndexExpr *>(expr);
  auto lhs = index->lhs;
  auto rhs = index->rhs;

  visitExpr(lhs);
  Node::Type *lhsType = createDuplicate(return_type.get());

  visitExpr(rhs);
  Node::Type *rhsType = createDuplicate(return_type.get());

  // search for '[]' in the lhs type and 'int' in the rhs type
  std::string lhsStr = type_to_string(lhsType);
  std::string rhsStr = type_to_string(rhsType);

  if (lhsType->kind != ND_ARRAY_TYPE) {
    std::string msg = "Indexing requires the left hand side to be an array but got '" + lhsStr + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  if (!isIntBasedType(rhsType)) {
    std::string msg = "Indexing requires the right hand side to be an 'int' but got '" + rhsStr + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
    return;
  }

  // set the return type to the underlying type of the array
  return_type = share(static_cast<ArrayType *>(lhsType)->underlying);
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
    if (at->constSize < 1) {
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
  return_type = share(cast->castee_type);
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
  size_t struct_size = 0;
  for (auto it : context->structTable[struct_name]) struct_size++;

  // We do not need ti check the struct size if the struct expression is empty
  if (struct_expr->values.empty()) {
    return_type = std::make_shared<SymbolType>(struct_name);
    struct_expr->asmType = new SymbolType(struct_name);
    return;
  }

  if (struct_size != struct_expr->values.size()) {
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
    Node::Type *elem_type = nullptr;
    // set elem_type to the type of the struct element, if it exists
    for (const auto& member :
         context->structTable[struct_name]) {
      if (member.first == elem.first) {
        elem_type = member.second.first;
        break;
      }
    }

    if (elem_type == nullptr) {
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
    if (elem.second->kind == ND_STRUCT) { // The work of typechecking the little guy has already been done for us
      return_type = share(elem_type);
      struct_expr->asmType = createDuplicate(return_type.get());
      return;
    }

    visitExpr(elem.second);
    // check for enums
    if (elem.second->kind == ND_MEMBER) {
      MemberExpr *member = static_cast<MemberExpr *>(elem.second);
      if (member->lhs->kind == ND_IDENT) {
        IdentExpr *ident = static_cast<IdentExpr *>(member->lhs);
        if (context->enumTable.find(ident->name) != context->enumTable.end()) {
          processEnumMember(member, ident->name); // Will automatically set return_type
          member->asmType = new SymbolType(ident->name); // maybe this works idrk lmao
        }
      }
    }
    Node::Type *expected = context->structTable[type_to_string(elem_type)][elem.first].first;
    if (checkTypeMatch(return_type.get(), expected)) {
      std::string msg = "Struct element '" + elem.first + "' requires type '" +
                        type_to_string(expected) + "' but got '" + type_to_string(return_type.get()) + "'";
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
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Alloc requires the number of bytes to be of type 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(alloc->line, alloc->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<PointerType>(new SymbolType("void"));
  // asmtype is a constant void* and already handled in ast
}

void TypeChecker::visitFreeMemory(Node::Expr *expr) {
  FreeMemoryExpr *freeMemory = static_cast<FreeMemoryExpr *>(expr);
  visitExpr(freeMemory->whatToFree);
  if (return_type.get()->kind != ND_POINTER_TYPE) {
    std::string msg = "Freeing memory requires the memory to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(freeMemory->line, freeMemory->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(freeMemory->bytesToFree);
  if (!isIntBasedType(return_type.get())) {
    std::string msg = "Freeing memory requires the number of bytes to be of type 'int' but got '" + type_to_string(return_type.get()) + "'";
    handleError(freeMemory->line, freeMemory->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
  // asmtype, once again, already handled
}

void TypeChecker::visitSizeof(Node::Expr *expr) {
  SizeOfExpr *sizeOf = static_cast<SizeOfExpr *>(expr);
  visitExpr(sizeOf->whatToSizeOf);
  return_type = std::make_shared<SymbolType>("int"); // This is always positive, but less than the max value, therefore it can be passed off as 'inferred' type.
  // asmtype is a constant int and already handled
}

void TypeChecker::visitMemcpyMemory(Node::Expr *expr) {
  MemcpyExpr *memcpy = static_cast<MemcpyExpr *>(expr);
  visitExpr(memcpy->dest);
  if (return_type.get()->kind != ND_POINTER_TYPE) {
    std::string msg = "Memcpy requires the destination to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(memcpy->line, memcpy->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(memcpy->src);
  if (return_type.get()->kind != ND_POINTER_TYPE) {
    std::string msg = "Memcpy requires the source to be of pointer type but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(memcpy->line, memcpy->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown"); // allows stuff to be fucked up later down the line hehe (even though the return type never changes)
    // asmtype is a constant int and already handled
  }

  visitExpr(memcpy->bytes);
  if (!isIntBasedType(return_type.get())) {
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

  return_type = std::make_shared<SymbolType>("int"); // Just like before, the fd is always positive but less than the limit, so it can be inferred
  // asmtype is a constant int and already handled
}