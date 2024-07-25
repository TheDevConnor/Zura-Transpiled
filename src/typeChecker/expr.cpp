#include "type.hpp"

void TypeChecker::visitNumber(callables_table &ctable, symbol_table &table,
                              Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  return_type = new SymbolType("int");
}

void TypeChecker::visitString(callables_table &ctable, symbol_table &table,
                              Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = new SymbolType("str");
}

void TypeChecker::visitIdent(callables_table &ctable, symbol_table &table,
                             Node::Expr *expr) {
  auto identifier = static_cast<IdentExpr *>(expr);
  return_type =
      table_lookup(table, identifier->name, identifier->line, identifier->pos);
}

void TypeChecker::visitBinary(callables_table &ctable, symbol_table &table,
                              Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(ctable, table, binary->lhs);
  auto lhs_type = return_type;

  visitExpr(ctable, table, binary->rhs);
  auto rhs_type = return_type;

  if (type_to_string(lhs_type) != type_to_string(rhs_type)) {
    std::string msg = "Binary operation expects both sides to be of the same "
                      "type but got " + type_to_string(lhs_type) + " and " +
                      type_to_string(rhs_type);
    handlerError(binary->line, binary->pos, msg, "");
  }
}

void TypeChecker::visitCall(callables_table &ctable, symbol_table &table,
                            Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  visitExpr(ctable, table, call->callee);

  auto params =
      table_lookup(ctable, static_cast<IdentExpr *>(call->callee)->name,
                   call->line, call->pos);

  if (params.size() != call->args.size()) {
    std::string msg = "Function '" +
                      static_cast<IdentExpr *>(call->callee)->name +
                      "' expects " + std::to_string(params.size()) +
                      " arguments but got " + std::to_string(call->args.size());
    handlerError(call->line, call->pos, msg, "");
  }

  for (int i = 0; i < params.size(); i++) {
    visitExpr(ctable, table, call->args[i]);
    if (type_to_string(params[i].second) != type_to_string(return_type)) {
      std::string msg = "Function '" +
                        static_cast<IdentExpr *>(call->callee)->name +
                        "' expects argument " + std::to_string(i) +
                        " to be of type " + type_to_string(params[i].second) +
                        " but got " + type_to_string(return_type);
      handlerError(call->line, call->pos, msg, "");
    }
  }

  return_type =
      table_lookup(table, static_cast<IdentExpr *>(call->callee)->name,
                   call->line, call->pos);
}

void TypeChecker::visitUnary(callables_table &ctable, symbol_table &table,
                             Node::Expr *expr) {
  auto unary = static_cast<UnaryExpr *>(expr);
  visitExpr(ctable, table, unary->expr);

  if (type_to_string(return_type) != "int") {
    std::string msg = "Unary operation expects an 'int' but got '" +
                      type_to_string(return_type) + "'";
    handlerError(unary->line, unary->pos, msg, "");
  }
}

void TypeChecker::visitGrouping(callables_table &ctable, symbol_table &table,
                                Node::Expr *expr) {
  auto grouping = static_cast<GroupExpr *>(expr);
  visitExpr(ctable, table, grouping->expr);
  return_type = return_type;
}