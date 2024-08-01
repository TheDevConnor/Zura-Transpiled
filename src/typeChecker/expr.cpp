#include "type.hpp"

void TypeChecker::visitExpr(Maps *map, Node::Expr *expr) {
  ExprAstLookup(expr, map);
}

void TypeChecker::visitNumber(Maps *map, Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  // TODO: check if the number is within the range a specific intege and if it
  // is a float
  return_type = new SymbolType("int");
}

void TypeChecker::visitString(Maps *map, Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = new SymbolType("str");
}

void TypeChecker::visitIdent(Maps *map, Node::Expr *expr) {
  auto ident = static_cast<IdentExpr *>(expr);
  return_type = Maps::lookup(map->local_symbol_table, ident->name, ident->line,
                             ident->pos, "local symbol table");
}

void TypeChecker::visitBinary(Maps *map, Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(map, binary->lhs);
  auto lhs = return_type;
  visitExpr(map, binary->rhs);
  auto rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = new SymbolType("unknown");
    return;
  }

  if (type_to_string(lhs) != type_to_string(rhs)) {
    std::string msg = "Binary operation '" + binary->op +
                      "' requires both sides to be the same type";
    handlerError(binary->line, binary->pos, msg, "", "Type Error");
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
    if (type_to_string(return_type) != type_to_string(fn.second[i].second)) {
      std::string msg = "Function '" + name->name + "' expects argument '" +
                        fn.second[i].first + "' to be a '" +
                        type_to_string(fn.second[i].second) + "' but got '" +
                        type_to_string(return_type) + "'";
      handlerError(call->line, call->pos, msg, "", "Type Error");
    }
  }

  return_type = fn.first.second;
}