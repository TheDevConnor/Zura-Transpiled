#include "type.hpp"

void TypeChecker::visitExpr(Maps::global_symbol_table &gTable,
                            Maps::local_symbol_table &lTable,
                            Maps::function_table &fn_table, Node::Expr *expr) {
  ExprAstLookup(expr, gTable, lTable, fn_table);
}

void TypeChecker::visitNumber(Maps::global_symbol_table &gTable,
                              Maps::local_symbol_table &lTable,
                              Maps::function_table &fn_table,
                              Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  // TODO: check if the number is within the range a specific intege and if it
  // is a float
  return_type = new SymbolType("int");
}

void TypeChecker::visitString(Maps::global_symbol_table &gTable,
                              Maps::local_symbol_table &lTable,
                              Maps::function_table &fn_table,
                              Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = new SymbolType("str");
}

void TypeChecker::visitIdent(Maps::global_symbol_table &gTable,
                             Maps::local_symbol_table &lTable,
                             Maps::function_table &fn_table, Node::Expr *expr) {
  auto ident = static_cast<IdentExpr *>(expr);
  auto res = (!Maps::lookup(lTable, ident->name, ident->line, ident->pos,
                            "local table"))
                 ? Maps::lookup(gTable, ident->name, ident->line, ident->pos,
                                "local table")
                 : Maps::lookup(lTable, ident->name, ident->line, ident->pos,
                                "global table");
  return_type = res;
}

void TypeChecker::visitBinary(Maps::global_symbol_table &gTable,
                              Maps::local_symbol_table &lTable,
                              Maps::function_table &fn_table,
                              Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(gTable, lTable, fn_table, binary->lhs);
  auto lhs = return_type;
  visitExpr(gTable, lTable, fn_table, binary->rhs);
  auto rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = new SymbolType("unknown");
    return;
  }

  if (type_to_string(lhs) != type_to_string(rhs)) {
    std::string msg = "Binary operation '" + binary->op +
                      "' requires both sides to be the same type";
    handlerError(binary->line, binary->pos, msg, "", "Type Error");
    return_type = new SymbolType("unknown");
    return;
  }

  return_type = lhs;
}

void TypeChecker::visitCall(Maps::global_symbol_table &gTable,
                            Maps::local_symbol_table &lTable,
                            Maps::function_table &fn_table, Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  auto name = static_cast<IdentExpr *>(call->callee);
  auto fn = Maps::lookup_fn(fn_table, name->name, call->line, call->pos);

  if (fn.second.size() != call->args.size()) {
    std::string msg = "Function '" + name->name + "' expects " +
                      std::to_string(fn.second.size()) + " arguments but got " +
                      std::to_string(call->args.size());
    handlerError(call->line, call->pos, msg, "", "Type Error");
    return_type = new SymbolType("unknown");
    return;
  }
}