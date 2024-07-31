#include "type.hpp"

void TypeChecker::visitExpr(global_symbol_table &gTable,
                            local_symbol_table &lTable, Node::Expr *expr) {
  ExprAstLookup(expr, gTable, lTable);
}

void TypeChecker::visitNumber(global_symbol_table &gTable,
                              local_symbol_table &lTable, Node::Expr *expr) {
  auto number = static_cast<NumberExpr *>(expr);
  // TODO: check if the number is within the range a specific integer
  return_type = new SymbolType("int");
}

void TypeChecker::visitString(global_symbol_table &gTable,
                              local_symbol_table &lTable, Node::Expr *expr) {
  auto string = static_cast<StringExpr *>(expr);
  return_type = new SymbolType("str");
}

void TypeChecker::visitIdent(TypeChecker::global_symbol_table &gTable,
                             local_symbol_table &lTable, Node::Expr *expr) {
  auto ident = static_cast<IdentExpr *>(expr);
  auto res = (lTable.find(ident->name) != lTable.end()) ? lTable[ident->name]
                                                        : gTable[ident->name];
  return_type = res;

  if (res == nullptr) {
    std::string msg =
        "'" + ident->name + "' is not defined in the local or global table";
    handlerError(ident->line, ident->pos, msg, "", "Symbol Table Error");
    return_type = new SymbolType("unknown");
  }
}

void TypeChecker::visitBinary(global_symbol_table &gTable, local_symbol_table &lTable, Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(gTable, lTable, binary->lhs);
  auto lhs = return_type;
  visitExpr(gTable, lTable, binary->rhs);
  auto rhs = return_type;

  if (lhs == nullptr || rhs == nullptr) {
    return_type = new SymbolType("unknown");
    return;
  }

  if (type_to_string(lhs) != type_to_string(rhs)) {
    std::string msg = "Binary operation '" + binary->op + "' requires both sides to be the same type";
    handlerError(binary->line, binary->pos, msg, "", "Type Error");
    return_type = new SymbolType("unknown");
    return;
  }

  return_type = lhs;
}