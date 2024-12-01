#include "../helper/math/math.hpp"
#include "type.hpp"
#include "../ast/types.hpp"
#include "../ast/ast.hpp"
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
  handlerError(template_call->line, template_call->pos, msg, "", "Type Error");
}

void TypeChecker::visitExternalCall(Maps *map, Node::Expr *expr) {
  ExternalCall *external_call = static_cast<ExternalCall *>(expr);

  return_type = std::make_shared<SymbolType>("unknown"); // Unknown type, imagine that this is cast to like int or whatever
  expr->asmType = return_type.get();
};

void TypeChecker::visitInt(Maps *map, Node::Expr *expr) {
  IntExpr *integer = static_cast<IntExpr *>(expr);

  // check if the integer is within the range of an i64 int
  if (integer->value > std::numeric_limits<signed long long int>::max() ||
      integer->value < std::numeric_limits<signed long long int>::min()) {
    std::string msg = "Integer '" + std::to_string(integer->value) +
                      "' is out of range for an 'int' which is 64 bits";
    handlerError(integer->line, integer->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
  expr->asmType = return_type.get();
}

void TypeChecker::visitFloat(Maps *map, Node::Expr *expr) {
  FloatExpr *floating = static_cast<FloatExpr *>(expr);

  // check if the float is within the range of an f32 float
  if (floating->value > std::numeric_limits<float>::max() ||
      floating->value < std::numeric_limits<float>::min()) {
    std::string msg = "Float '" + std::to_string(floating->value) +
                      "' is out of range for a 'float' which is 32 bits";
    handlerError(floating->line, floating->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("float");
  expr->asmType = return_type.get();
}

void TypeChecker::visitString(Maps *map, Node::Expr *expr) {
  StringExpr *string = static_cast<StringExpr *>(expr);
  return_type = std::make_shared<SymbolType>("str");
  expr->asmType = return_type.get();
}

void TypeChecker::visitIdent(Maps *map, Node::Expr *expr) {
  IdentExpr *ident = static_cast<IdentExpr *>(expr);
  Node::Type *res = nullptr;

  if (map->local_symbol_table.find(ident->name) != map->local_symbol_table.end()) {
    res = map->local_symbol_table[ident->name];
  } else if (map->global_symbol_table.find(ident->name) != map->global_symbol_table.end()) {
    res = map->global_symbol_table[ident->name];
  }
  
  // check if we found something in the local symbol table if not return error
  // of 'did you mean'
  if (res == nullptr) {
    for (std::pair<std::string, Node::Type *> pair : map->local_symbol_table)
      map->stackKeys.push_back(pair.first);
    std::optional<std::string> closest = string_distance(map->stackKeys, ident->name, 3);

    std::string msg = "Undefined variable '" + ident->name + "' did you mean '" + closest.value() + "'?";
    handlerError(ident->line, ident->pos, msg, "", "Symbol Table Error");
    res = new SymbolType("unknown"); // return unknown type
  }

  // update the ast-node (IdentExpr) to hold the type of the identifier as a
  // property
  ident->type = expr->asmType = res;
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
        expr->asmType = return_type.get();
        return;
    }

    if (map->mathOps.find(binary->op) != map->mathOps.end()) {
      return_type = lhs;
      expr->asmType = lhs.get();
    } else if (map->boolOps.find(binary->op) != map->boolOps.end()) {
      return_type = std::make_shared<SymbolType>("bool");
      expr->asmType = return_type.get();
    } else {
      std::string msg = "Unsupported binary operator: " + binary->op;
      handlerError(binary->line, binary->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      expr->asmType = return_type.get();
    }

}

void TypeChecker::visitUnary(Maps *map, Node::Expr *expr) {
  UnaryExpr *unary = static_cast<UnaryExpr *>(expr);
  visitExpr(map, unary->expr);

  return_type = checkReturnType(expr, "unknown");

  if (unary->op == "-" && type_to_string(return_type.get()) != "int") {
    std::string msg = "Unary operation '" + unary->op +
                      "' requires the type to be an 'int' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(unary->line, unary->pos, msg, "", "Type Error");
  }

  if (unary->op == "!" && type_to_string(return_type.get()) != "bool") {
    std::string msg = "Unary operation '" + unary->op +
                      "' requires the type to be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(unary->line, unary->pos, msg, "", "Type Error");
  }

  // check for ++ and --
  if (unary->op == "++" || unary->op == "--") {
    if (type_to_string(return_type.get()) != "int") {
      std::string msg = "Unary operation '" + unary->op +
                        "' requires the type to be an 'int' but got '" +
                        type_to_string(return_type.get()) + "'";
      handlerError(unary->line, unary->pos, msg, "", "Type Error");
    }
  }

  expr->asmType = return_type.get();
}

void TypeChecker::visitGrouping(Maps *map, Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);
  visitExpr(map, grouping->expr);
  return_type = return_type;
  expr->asmType = return_type.get();
}

void TypeChecker::visitBool(Maps *map, Node::Expr *expr) {
  BoolExpr *boolean = static_cast<BoolExpr *>(expr);
  return_type = std::make_shared<SymbolType>("bool");
  expr->asmType = return_type.get();
}

void TypeChecker::visitTernary(Maps *map, Node::Expr *expr) {
  TernaryExpr *ternary = static_cast<TernaryExpr *>(expr);
  visitExpr(map, ternary->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "Ternary condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(ternary->line, ternary->pos, msg, "", "Type Error");
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
    handlerError(ternary->line, ternary->pos, msg, "", "Type Error");
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

  if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
    std::string msg =
        "Assignment requires both sides to be the same type and got '" +
        type_to_string(lhs.get()) + "' and '" + type_to_string(rhs.get()) + "'";
    handlerError(assign->line, assign->pos, msg, "", "Type Error");
  }

  return_type = lhs;
  expr->asmType = lhs.get();
}

void TypeChecker::visitCall(Maps *map, Node::Expr *expr) {
    auto call = static_cast<CallExpr *>(expr);
    auto name = static_cast<IdentExpr *>(call->callee);

    // Lookup function
    auto fn = lookup_fn(map, name->name, call->line, call->pos);

    if (fn.empty()) {
        return; // Function not found, error already handled in lookup_fn
    }

    if (fn.size() > 1) {
        reportOverloadedFunctionError(call, name->name);
        return;
    }

    auto [fnName, fnParams] = fn[0];

    if (!validateArgumentCount(call, name->name, fnParams)) {
        return;
    }

    if (!validateArgumentTypes(map, call, name->name, fnParams)) {
        return;
    }

    // Set return type based on the first parameter's type (assuming standard convention)
    return_type = std::make_shared<SymbolType>(type_to_string(fnParams[0].second));
    expr->asmType = return_type.get();
}

void TypeChecker::visitMember(Maps *map, Node::Expr *expr) {
    MemberExpr *member = static_cast<MemberExpr *>(expr);

    // Verify that the lhs is a struct or enum
    visitExpr(map, member->lhs);
    std::string lhsType = type_to_string(return_type.get());
    std::string name = static_cast<IdentExpr *>(member->lhs)->name;
    // Determine if lhs is a struct or enum
    std::string typeKind = determineTypeKind(map, name);

    if (typeKind == "struct") {
        processStructMember(map, member, name);
    } else if (typeKind == "enum") {
        processEnumMember(map, member, name);
    } else {
        handleUnknownType(member, name);
    }
}

void TypeChecker::visitArray(Maps *map, Node::Expr *expr) {
  ArrayExpr *array = static_cast<ArrayExpr *>(expr);
  for (Node::Expr *elem : array->elements) {
    switch (elem->kind) {
    case NodeKind::ND_INT:
      map->array_table.push_back(new SymbolType("int"));
      break;
    case NodeKind::ND_FLOAT:
      map->array_table.push_back(new SymbolType("float"));
      break;
    case NodeKind::ND_STRING:
      map->array_table.push_back(new SymbolType("str"));
      break;
    case NodeKind::ND_BOOL:
      map->array_table.push_back(new SymbolType("bool"));
      break;
    default:
      visitExpr(map, elem);
      break;
    }

    // check if the type of the element is the same as the previous element
    if (map->array_table.size() > 1) {
      if (type_to_string(map->array_table[0]) !=
          type_to_string(map->array_table[map->array_table.size() - 1])) {
        std::string msg =
            "Array elements must be of the same type but got '" +
            type_to_string(map->array_table[0]) + "' and '" +
            type_to_string(map->array_table[map->array_table.size() - 1]) + "'";
        handlerError(array->line, array->pos, msg, "", "Type Error");
      }
    }
  }
  return_type =
      std::make_shared<ArrayType>(map->array_table[0]);
  expr->asmType = return_type.get();

  // clear the array table
  map->array_table.clear();
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
    handlerError(index->line, index->pos, msg, "", "Type Error");
  }

  if (type_to_string(array.get()).find("[]") == std::string::npos) {
    std::string msg =
        "Indexing requires the array to be of type '[]' but got '" +
        type_to_string(array.get()) + "'";
    handlerError(index->line, index->pos, msg, "", "Type Error");
  }

  return_type =
      std::make_shared<SymbolType>(type_to_string(array.get()).substr(2));
}

void TypeChecker::visitCast(Maps *map, Node::Expr *expr) {
  // Cast the generic expression to a CastExpr
  CastExpr *cast = static_cast<CastExpr *>(expr);

  expr->asmType = cast->castee_type;
  return_type = std::make_shared<SymbolType>(type_to_string(cast->castee_type));
}
