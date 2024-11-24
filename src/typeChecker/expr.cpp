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
  auto template_call = static_cast<TemplateCallExpr *>(expr);

  auto msg = "Template function calls are not supported yet";
  handlerError(template_call->line, template_call->pos, msg, "", "Type Error");
}

void TypeChecker::visitExternalCall(Maps *map, Node::Expr *expr) {
  auto external_call = static_cast<ExternalCall *>(expr);

  return_type = std::make_shared<SymbolType>("unknown"); // Unknown type, imagine that this is cast to like int or whatever
};

void TypeChecker::visitInt(Maps *map, Node::Expr *expr) {
  auto integer = static_cast<IntExpr *>(expr);

  // check if the integer is within the range of an i64 int
  if (integer->value > std::numeric_limits<signed long long int>::max() ||
      integer->value < std::numeric_limits<signed long long int>::min()) {
    std::string msg = "Integer '" + std::to_string(integer->value) +
                      "' is out of range for an 'int' which is 64 bits";
    handlerError(integer->line, integer->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("int");
}

void TypeChecker::visitFloat(Maps *map, Node::Expr *expr) {
  auto floating = static_cast<FloatExpr *>(expr);

  // check if the float is within the range of an f32 float
  if (floating->value > std::numeric_limits<float>::max() ||
      floating->value < std::numeric_limits<float>::min()) {
    std::string msg = "Float '" + std::to_string(floating->value) +
                      "' is out of range for a 'float' which is 32 bits";
    handlerError(floating->line, floating->pos, msg, "", "Type Error");
  }

  return_type = std::make_shared<SymbolType>("float");
}

void TypeChecker::visitString(Maps *map, Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = std::make_shared<SymbolType>("str");
}

void TypeChecker::visitIdent(Maps *map, Node::Expr *expr) {
  auto ident = static_cast<IdentExpr *>(expr);
  Node::Type *res = nullptr;

  if (map->local_symbol_table.find(ident->name) != map->local_symbol_table.end()) {
    res = map->local_symbol_table[ident->name];
  } else if (map->global_symbol_table.find(ident->name) != map->global_symbol_table.end()) {
    res = map->global_symbol_table[ident->name];
  }
  
  // check if we found something in the local symbol table if not return error
  // of 'did you mean'
  if (res == nullptr) {
    for (auto const &[key, _] : map->local_symbol_table)
      map->stackKeys.push_back(key);
    std::optional<std::string> closest = string_distance(map->stackKeys, ident->name, 3);

    std::string msg = "Undefined variable '" + ident->name + "' did you mean '" + closest.value() + "'?";
    handlerError(ident->line, ident->pos, msg, "", "Symbol Table Error");
    res = new SymbolType("unknown"); // return unknown type
  }

  // update the ast-node (IdentExpr) to hold the type of the identifier as a
  // property
  ident->type = ident->asmType = res;
  return_type = std::make_shared<SymbolType>(type_to_string(res));
}

void TypeChecker::visitBinary(Maps *map, Node::Expr *expr) {
    auto binary = static_cast<BinaryExpr *>(expr);

    visitExpr(map, binary->lhs);
    auto lhs = checkReturnType(binary->lhs, "unknown");
    visitExpr(map, binary->rhs);
    auto rhs = checkReturnType(binary->rhs, "unknown");

    if (!checkTypeMatch(lhs, rhs, binary->op, binary->line, binary->pos)) {
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    if (map->mathOps.find(binary->op) != map->mathOps.end()) {
        return_type = lhs;
    } else if (map->boolOps.find(binary->op) != map->boolOps.end()) {
        return_type = std::make_shared<SymbolType>("bool");
    } else {
        std::string msg = "Unsupported binary operator: " + binary->op;
        handlerError(binary->line, binary->pos, msg, "", "Type Error");
    }

    expr->asmType = return_type.get();
}

void TypeChecker::visitUnary(Maps *map, Node::Expr *expr) {
  auto unary = static_cast<UnaryExpr *>(expr);
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
  auto grouping = static_cast<GroupExpr *>(expr);
  visitExpr(map, grouping->expr);
  return_type = return_type;
  expr->asmType = return_type.get();
}

void TypeChecker::visitBool(Maps *map, Node::Expr *expr) {
  auto boolean = static_cast<BoolExpr *>(expr);
  return_type = std::make_shared<SymbolType>("bool");
}

void TypeChecker::visitTernary(Maps *map, Node::Expr *expr) {
  auto ternary = static_cast<TernaryExpr *>(expr);
  visitExpr(map, ternary->condition);
  if (type_to_string(return_type.get()) != "bool") {
    std::string msg = "Ternary condition must be a 'bool' but got '" +
                      type_to_string(return_type.get()) + "'";
    handlerError(ternary->line, ternary->pos, msg, "", "Type Error");
  }

  visitExpr(map, ternary->lhs);
  auto lhs = return_type;
  visitExpr(map, ternary->rhs);
  auto rhs = return_type;

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
}

void TypeChecker::visitAssign(Maps *map, Node::Expr *expr) {
  auto assign = static_cast<AssignmentExpr *>(expr);
  visitExpr(map, assign->assignee);
  auto lhs = return_type;
  visitExpr(map, assign->rhs);
  auto rhs = return_type;

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
}

void TypeChecker::visitCall(Maps *map, Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  auto name = static_cast<IdentExpr *>(call->callee);
  auto fn = lookup_fn(map, name->name, call->line, call->pos);
  
  if (fn.size() == 0) {
    return;
  }

  if (fn.size() > 1) {
    std::string msg = "Function '" + name->name + "' is overloaded";
    handlerError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  auto [fn_name, fn_params] = fn[0];
  if (fn_params.size() != call->args.size()) {
    std::string msg = "Function '" + name->name + "' requires " +
                      std::to_string(fn_params.size()) + " arguments but got " +
                      std::to_string(call->args.size());
    handlerError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  for (size_t i = 0; i < call->args.size(); i++) {
    visitExpr(map, call->args[i]);
    if (type_to_string(return_type.get()) != type_to_string(fn_params[i].second)) {
      std::string msg = "Function '" + name->name + "' requires argument '" +
                        fn_params[i].first + "' to be of type '" +
                        type_to_string(fn_params[i].second) + "' but got '" +
                        type_to_string(return_type.get()) + "'";
      handlerError(call->line, call->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return;
    }
  }

  return_type = std::make_shared<SymbolType>(type_to_string(fn_params[0].second));
  expr->asmType = return_type.get();
}

void TypeChecker::visitMember(Maps *map, Node::Expr *expr) {
    auto member = static_cast<MemberExpr *>(expr);

    // Verify that the lhs is a struct or enum
    visitExpr(map, member->lhs);
    auto lhsType = type_to_string(return_type.get());

    // Determine if lhs is a struct or enum
    auto typeKind = determineTypeKind(map, lhsType);

    if (typeKind == "struct") {
        processStructMember(map, member, lhsType);
    } else if (typeKind == "enum") {
        processEnumMember(map, member, lhsType);
    } else {
        handleUnknownType(member, lhsType);
    }
}

void TypeChecker::visitArray(Maps *map, Node::Expr *expr) {
  auto array = static_cast<ArrayExpr *>(expr);
  for (auto &elem : array->elements) {
    // push the type of the element into the array table
    if (elem->kind == NodeKind::ND_INT) {
      auto number = static_cast<IntExpr *>(elem);
      map->array_table.push_back(new SymbolType("int"));
    } else if (elem->kind == NodeKind::ND_STRING) {
      map->array_table.push_back(new SymbolType("str"));
    } else if (elem->kind == NodeKind::ND_BOOL) {
      map->array_table.push_back(new SymbolType("bool"));
    } else if (elem->kind == NodeKind::ND_FLOAT) {
      map->array_table.push_back(new SymbolType("float"));
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
      std::make_shared<SymbolType>("[]" + type_to_string(map->array_table[0]));

  // clear the array table
  map->array_table.clear();
}

void TypeChecker::visitIndex(Maps *map, Node::Expr *expr) {
  auto index = static_cast<IndexExpr *>(expr);
  visitExpr(map, index->lhs);
  auto array = return_type;
  visitExpr(map, index->rhs);
  auto idx = return_type;

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
  auto cast = static_cast<CastExpr *>(expr);

  expr->asmType = cast->castee_type;
  return_type = std::make_shared<SymbolType>(type_to_string(cast->castee_type));
}
