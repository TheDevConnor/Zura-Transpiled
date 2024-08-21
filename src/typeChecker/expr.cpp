#include "type.hpp"
#include <memory>

void TypeChecker::visitExpr(Maps *map, Node::Expr *expr) {
  ExprAstLookup(expr, map);
}

void TypeChecker::visitNumber(Maps *map, Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  // TODO: check if the number is within the range a specific intege and if it
  // is a float
  return_type = std::make_shared<SymbolType>("int");
}

void TypeChecker::visitString(Maps *map, Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = std::make_shared<SymbolType>("str");
}

void TypeChecker::visitIdent(Maps *map, Node::Expr *expr) {
  auto ident = static_cast<IdentExpr *>(expr);
  auto res = Maps::lookup(map->local_symbol_table, ident->name, ident->line,
                          ident->pos, "local symbol table");
  return_type = std::make_shared<SymbolType>(type_to_string(res));
}

void TypeChecker::visitBinary(Maps *map, Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(map, binary->lhs);
  auto lhs = return_type;
  visitExpr(map, binary->rhs);
  auto rhs = return_type;

  // validate the types of the lhs and rhs
  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  // check if the op is of type bool aka +, -, *, /
  std::vector<std::string> math_ops = {"+", "-", "*", "/", "%", "^"};
  if (std::find(math_ops.begin(), math_ops.end(), binary->op) !=
      math_ops.end()) {
    if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
      std::string msg = "Binary operation '" + binary->op +
                        "' requires both sides to be the same type";
      handlerError(binary->line, binary->pos, msg, "", "Type Error");
    }
    return_type = lhs;
    return;
  }

  // check if the op is of type bool aka >, <, >=, <=, ==, !=
  std::vector<std::string> bool_ops = {">", "<", ">=", "<=", "==", "!="};
  if (std::find(bool_ops.begin(), bool_ops.end(), binary->op) !=
      bool_ops.end()) {
    if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
      std::string msg = "Binary operation '" + binary->op +
                        "' requires both sides to be of type '" +
                        type_to_string(lhs.get()) + "' but got '" +
                        type_to_string(rhs.get()) + "'";
      handlerError(binary->line, binary->pos, msg, "", "Type Error");
    }
    return_type = std::make_shared<SymbolType>("bool");
    return;
  }
}

void TypeChecker::visitUnary(Maps *map, Node::Expr *expr) {
  auto unary = static_cast<UnaryExpr *>(expr);
  visitExpr(map, unary->expr);

  if (return_type == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

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

  return_type = return_type;
}

void TypeChecker::visitGrouping(Maps *map, Node::Expr *expr) {
  auto grouping = static_cast<GroupExpr *>(expr);
  visitExpr(map, grouping->expr);
  return_type = return_type;
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
    std::string msg = "Assignment requires both sides to be the same type";
    handlerError(assign->line, assign->pos, msg, "", "Type Error");
  }

  return_type = lhs;
}

void TypeChecker::visitCall(Maps *map, Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  auto name = static_cast<IdentExpr *>(call->callee);
  auto fn = Maps::lookup_fn(map, name->name, call->line, call->pos);

  if (fn.second.size() != call->args.size()) {
    std::string msg = "Function '" + name->name + "' expects " +
                      std::to_string(fn.second.size()) + " arguments but got " +
                      std::to_string(call->args.size());
    handlerError(call->line, call->pos, msg, "", "Type Error");
  }

  for (int i = 0; i < call->args.size(); i++) {
    visitExpr(map, call->args[i]);
    if (type_to_string(return_type.get()) !=
        type_to_string(fn.second[i].second)) {
      std::string msg = "Function '" + name->name + "' expects argument '" +
                        fn.second[i].first + "' to be of type '" +
                        type_to_string(fn.second[i].second) + "' but got '" +
                        type_to_string(return_type.get()) + "'";
      handlerError(call->line, call->pos, msg, "", "Type Error");
    }
  }

  return_type = std::make_shared<SymbolType>(type_to_string(fn.first.second));
}

void TypeChecker::visitMember(Maps *map, Node::Expr *expr) {
  std::cout << "Not implemented yet" << std::endl;
}