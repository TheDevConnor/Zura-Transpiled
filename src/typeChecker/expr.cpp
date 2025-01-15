#include "../ast/ast.hpp"
#include "../ast/types.hpp"
#include "../helper/math/math.hpp"
#include "type.hpp"
#include <memory>

#include <limits>
#include <optional>
#include <string>

void TypeChecker::visitExpr(Maps *map, Node::Expr *expr) {
  ExprAstLookup(expr, map);
}

void TypeChecker::visitTemplateCall(Maps *map, Node::Expr *expr) {
  TemplateCallExpr *template_call = static_cast<TemplateCallExpr *>(expr);

  std::string msg = "Template function calls are not supported yet";
  handleError(template_call->line, template_call->pos, msg, "", "Type Error");
}

void TypeChecker::visitExternalCall(Maps *map, Node::Expr *expr) {
  ExternalCall *external_call = static_cast<ExternalCall *>(expr);

  return_type = std::make_shared<SymbolType>(
      "unknown"); // Unknown type, imagine that this is cast to like int or
                  // whatever
  expr->asmType = new SymbolType("unknown");
};

void TypeChecker::visitInt(Maps *map, Node::Expr *expr) {
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

void TypeChecker::visitFloat(Maps *map, Node::Expr *expr) {
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

void TypeChecker::visitString(Maps *map, Node::Expr *expr) {
  StringExpr *string = static_cast<StringExpr *>(expr);
  return_type = std::make_shared<SymbolType>("str");
  expr->asmType = new SymbolType("str");
}

void TypeChecker::visitChar(Maps *map, Node::Expr *expr) {
  CharExpr *character = static_cast<CharExpr *>(expr);
  return_type = std::make_shared<SymbolType>("char");
  expr->asmType = new SymbolType("char");
}

void TypeChecker::visitAddress(Maps *map, Node::Expr *expr) {
  AddressExpr *address = static_cast<AddressExpr *>(expr);
  visitExpr(map, address->right);
  return_type = std::make_shared<PointerType>(return_type.get());
  expr->asmType = new PointerType(address->right->asmType);
}

void TypeChecker::visitIdent(Maps *map, Node::Expr *expr) {
  IdentExpr *ident = static_cast<IdentExpr *>(expr);
  Node::Type *res = nullptr;

  if (map->local_symbol_table.find(ident->name) !=
      map->local_symbol_table.end()) {
    res = map->local_symbol_table[ident->name];
    ident->asmType = createDuplicate(res);
  } else if (map->global_symbol_table.find(ident->name) !=
             map->global_symbol_table.end()) {
    res = map->global_symbol_table[ident->name];
    ident->asmType = createDuplicate(res);
  }

  // check if we found something in the local symbol table if not return error
  // of 'did you mean'
  if (res == nullptr) {
    for (std::pair<std::string, Node::Type *> pair : map->local_symbol_table)
      map->stackKeys.push_back(pair.first);
    std::optional<std::string> closest =
        string_distance(map->stackKeys, ident->name, 3);
    std::string msg = "";
    if (closest.has_value()) {
      msg = "Undefined variable '" + ident->name + "' did you mean '" +
            closest.value() + "'?";
    } else {
      msg = "Undefined variable '" + ident->name + "'";
    }
    handleError(ident->line, ident->pos, msg, "", "Type Error");
    res = new SymbolType("unknown"); // return unknown type
  }

  // update the ast-node (IdentExpr) to hold the type of the identifier as a
  // property
  ident->type = res;
  return_type = std::make_shared<SymbolType>(type_to_string(res));
}

void TypeChecker::visitBinary(Maps *map, Node::Expr *expr) {
  BinaryExpr *binary = static_cast<BinaryExpr *>(expr);

  visitExpr(map, binary->lhs);
  std::shared_ptr<SymbolType> lhs = checkReturnType(binary->lhs, "unknown");
  visitExpr(map, binary->rhs);
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

  if (map->mathOps.find(binary->op) != map->mathOps.end()) {
    return_type = lhs;
    expr->asmType =
        new SymbolType(lhs.get()->name); // This, for some reason, is needed,
                                         // and only in debug mode?????
  } else if (map->boolOps.find(binary->op) != map->boolOps.end()) {
    return_type = std::make_shared<SymbolType>("bool");
    expr->asmType = new SymbolType("bool");
  } else {
    std::string msg = "Unsupported binary operator: " + binary->op;
    handleError(binary->line, binary->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    expr->asmType = new SymbolType("unknown");
  }
}

void TypeChecker::visitUnary(Maps *map, Node::Expr *expr) {
  UnaryExpr *unary = static_cast<UnaryExpr *>(expr);
  visitExpr(map, unary->expr);

  return_type = checkReturnType(expr, "unknown");
  expr->asmType = createDuplicate(return_type.get());
  ;

  if (unary->op == "-" && type_to_string(return_type.get()) != "int") {
    std::string msg = "Unary operation '" + unary->op +
                      "' requires the type to be an 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(unary->line, unary->pos, msg, "", "Type Error");
  }

  if (unary->op == "!" && type_to_string(return_type.get()) != "bool") {
    std::string msg = "Unary operation '" + unary->op +
                      "' requires the type to be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(unary->line, unary->pos, msg, "", "Type Error");
  }

  // check for ++ and --
  if (unary->op == "++" || unary->op == "--") {
    if (type_to_string(return_type.get()) != "int") {
      std::string msg = "Unary operation '" + unary->op +
                        "' requires the type to be an 'int' but got '" +
                        type_to_string(return_type.get()) + "'";
      handleError(unary->line, unary->pos, msg, "", "Type Error");
    }
  }
}

void TypeChecker::visitGrouping(Maps *map, Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);
  visitExpr(map, grouping->expr);
  expr->asmType = createDuplicate(return_type.get());
  ;
}

void TypeChecker::visitBool(Maps *map, Node::Expr *expr) {
  BoolExpr *boolean = static_cast<BoolExpr *>(expr);
  return_type = std::make_shared<SymbolType>("bool");
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitTernary(Maps *map, Node::Expr *expr) {
  TernaryExpr *ternary = static_cast<TernaryExpr *>(expr);
  visitExpr(map, ternary->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "Ternary condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handleError(ternary->line, ternary->pos, msg, "", "Type Error");
  }

  visitExpr(map, ternary->lhs);
  std::shared_ptr<Node::Type> lhs = return_type;
  visitExpr(map, ternary->rhs);
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

void TypeChecker::visitAssign(Maps *map, Node::Expr *expr) {
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(expr);
  visitExpr(map, assign->assignee);
  std::shared_ptr<Node::Type> lhs = return_type;
  visitExpr(map, assign->rhs);
  std::shared_ptr<Node::Type> rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  // check if the lhs is a parameter of a function
  if (assign->assignee->kind == ND_IDENT) {
    IdentExpr *ident = static_cast<IdentExpr *>(assign->assignee);
    for (auto param : map->function_params) {
      if (param == ident->name) {
        std::string msg =
            "Cannot assign to a function parameter '" + param + "'";
        handleError(assign->line, assign->pos, msg, "", "Type Error");
      }
    }
  }

  std::string msg =
      "Assignment requires both sides to be the same type, but got '" +
      type_to_string(lhs.get()) + "' and '" + type_to_string(rhs.get()) + "'";
  checkTypeMatch(std::make_shared<SymbolType>(type_to_string(lhs.get())),
                 std::make_shared<SymbolType>(type_to_string(rhs.get())), "=",
                 assign->line, assign->pos, msg);

  return_type = lhs;
  expr->asmType = lhs.get();
}

void TypeChecker::visitCall(Maps *map, Node::Expr *expr) {
  CallExpr *call = static_cast<CallExpr *>(expr);
  Node::Expr *name = call->callee;

  // Lookup function
  FnVector fn = lookup_fn(map, name, call->line, call->pos);

  if (fn.empty()) {
    return; // Function not found, error already handled in lookup_fn
  }

  if (fn.size() > 1) {
    reportOverloadedFunctionError(call, name);
    return;
  }

  auto [fnName, fnParams] = fn[0];

  if (!validateArgumentCount(call, name, fnParams)) {
    return;
  }

  if (!validateArgumentTypes(map, call, name, fnParams)) {
    return;
  }

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
}

void TypeChecker::visitMember(Maps *map, Node::Expr *expr) {
  MemberExpr *member = static_cast<MemberExpr *>(expr);

  // Verify that the lhs is a struct or enum
  visitExpr(map, member->lhs);
  std::string lhsType = type_to_string(return_type.get());
  std::string name = "";
  if (member->lhs->kind == ND_MEMBER) {
    name = static_cast<IdentExpr *>(static_cast<MemberExpr *>(member->lhs)->rhs)
               ->name;
  } else {
    name = static_cast<IdentExpr *>(member->rhs)->name;
  }
  // Determine if lhs is a struct or enum
  std::string typeKind = determineTypeKind(map, lhsType);

  if (typeKind == "struct") {
    processStructMember(map, member, name, lhsType);
  } else if (typeKind == "enum") {
    processEnumMember(map, member, name);
  } else {
    handleUnknownType(member, name);
  }
}

void TypeChecker::visitArray(Maps *map, Node::Expr *expr) {
  ArrayExpr *array = static_cast<ArrayExpr *>(expr);
  if (array->type == nullptr) {
    array->type = new ArrayType(new SymbolType("unknown"), -1);
    expr->asmType = array->type;
  }

  // check if the array is empty
  if (array->elements.empty()) {
    std::string msg = "Array must have at least one element!";
    handleError(array->line, array->pos, msg, "", "Type Error");
    return_type = std::make_shared<ArrayType>(array->type, 0);
    expr->asmType = createDuplicate(return_type.get());
    return;
  }

  for (Node::Expr *elem : array->elements) {
    visitExpr(map, elem);
    if (return_type == nullptr) {
      return_type = std::make_shared<SymbolType>("unknown");
      expr->asmType = createDuplicate(return_type.get());
      return;
    }
    map->array_table.push_back(return_type.get());
  }

  if (map->array_table.empty()) {
    ArrayType *at = static_cast<ArrayType *>(array->type);
    return_type = std::make_shared<ArrayType>(at->underlying, 0);
    expr->asmType = createDuplicate(return_type.get());
    return;
  }
  return_type =
      std::make_shared<ArrayType>(map->array_table[0], map->array_table.size());
  expr->asmType = createDuplicate(return_type.get());

  // clear the array table
  map->array_table.clear();
}

void TypeChecker::visitArrayType(Maps *map, Node::Type *type) {
  ArrayType *array = static_cast<ArrayType *>(type);
  return_type =
      std::make_shared<ArrayType>(array->underlying, array->constSize);
}

void TypeChecker::visitIndex(Maps *map, Node::Expr *expr) {
  IndexExpr *index = static_cast<IndexExpr *>(expr);
  visitExpr(map, index->lhs);
  std::shared_ptr<Node::Type> array = return_type;
  visitExpr(map, index->rhs);
  std::shared_ptr<Node::Type> idx = return_type;

  if (type_to_string(idx.get()) != "int") {
    std::string msg = "Array index must be of type 'int' but got '" +
                      type_to_string(idx.get()) + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
  }

  if (array.get()->kind == ND_ARRAY_TYPE) {
    std::string msg =
        "Indexing requires the lhs to be of type array but got '" +
        type_to_string(array.get()) + "'";
    handleError(index->line, index->pos, msg, "", "Type Error");
  }

  return_type =
      std::make_shared<SymbolType>(type_to_string(array.get()).substr(2));
  expr->asmType = createDuplicate(return_type.get());
}

void TypeChecker::visitArrayAutoFill(Maps *map, Node::Expr *expr) {
  ArrayExpr *array = static_cast<ArrayExpr *>(expr);

  // confirm that we have an elem and that is it 0
  if (array->elements.size() != 1) {
    std::string msg =
        "Auto-fill arrays must have an explicitly-defined length.";
    handleError(array->line, array->pos, msg, "", "Type Error");
    return_type = std::make_shared<ArrayType>(new SymbolType("unknown"), 0);
    expr->asmType = createDuplicate(return_type.get());
    return;
  }
}

void TypeChecker::visitCast(Maps *map, Node::Expr *expr) {
  // Cast the generic expression to a CastExpr
  CastExpr *cast = static_cast<CastExpr *>(expr);
  // Visit the inside (update it's inner asmType)
  visitExpr(map, cast->castee);
  expr->asmType = cast->castee_type;
  return_type = std::make_shared<SymbolType>(type_to_string(cast->castee_type));
}

void TypeChecker::visitStructExpr(Maps *map, Node::Expr *expr) {
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
  if (map->struct_table.find(struct_name) == map->struct_table.end()) {
    std::string msg =
        "Struct '" + struct_name + "' is not defined in the scope.";
    handleError(struct_expr->line, struct_expr->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    struct_expr->asmType = new SymbolType("unknown");
    return;
  }

  // check if the number of elements in the struct is the same as the number of
  // elements in the struct expression
  int struct_size = 0;
  struct_size = map->struct_table[struct_name].size();

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
  for (std::pair<Node::Expr *, Node::Expr *> elem : struct_expr->values) {
    // find the type of the struct element
    IdentExpr *name = static_cast<IdentExpr *>(elem.first);
    std::string elem_type = "";
    // set elem_type to the type of the struct element, if it exists
    for (std::pair<std::string, Node::Type *> member :
         map->struct_table[struct_name]) {
      if (member.first == name->name) {
        elem_type = type_to_string(member.second);
        break;
      }
    }

    if (elem_type == "") {
      std::string msg = "Named member '" + name->name +
                        "' not found in struct '" + struct_name + "'";
      // did you mean?
      std::vector<std::string> known;
      for (std::pair<std::string, Node::Type *> member :
           map->struct_table[struct_name]) {
        known.push_back(member.first);
      }
      std::optional<std::string> closest =
          string_distance(known, name->name, 3);
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

    visitExpr(map, elem.second);
    // check for enums
    if (elem.second->kind == ND_MEMBER) {
      MemberExpr *member = static_cast<MemberExpr *>(elem.second);
      if (member->lhs->kind == ND_IDENT) {
        IdentExpr *ident = static_cast<IdentExpr *>(member->lhs);
        if (map->enum_table.find(ident->name) != map->enum_table.end()) {
          processEnumMember(map, member,
                            ident->name); // Will automatically set return_type
          member->asmType =
              createDuplicate(return_type.get()); // maybe this works idrk lmao
        }
      }
    }
    std::string expected = type_to_string(elem.second->asmType);
    if (checkReturnType(elem.second, expected) == nullptr) {
      std::string msg = "Struct element '" + name->name + "' requires type '" +
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
