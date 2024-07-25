#include "type.hpp"

void TypeChecker::visitExpr(global_symbol_table &gTable, local_symbol_table &lTable, Node::Expr *expr) {
    ExprAstLookup(expr, gTable, lTable);
}

void TypeChecker::visitNumber(global_symbol_table &gTable, local_symbol_table &lTable,
                              Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  // TODO: check if the number is within the range a specific integer
  return_type = new SymbolType("int");
}

void TypeChecker::visitString(global_symbol_table &gTable, local_symbol_table &lTable,
                              Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = new SymbolType("str");
}