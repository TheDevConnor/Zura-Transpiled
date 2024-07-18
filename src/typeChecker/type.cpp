#include "type.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"

void TypeChecker::check(Node::Stmt *stmt) {
  symbol_table table;
  callables_table ctable;
  visitStmt(ctable, table, stmt);
}

void TypeChecker::handlerError(int line, int pos, std::string msg) {
  Lexer lexer; // dummy lexer
  ErrorClass::error(line, pos, msg, "", "Type Error", "main.zu", lexer,
                    node.tks, false, false, true, false, true);
}

std::string TypeChecker::type_to_string(Node::Type *type) {
  switch (type->kind) {
  case NodeKind::ND_SYMBOL_TYPE:
    return static_cast<SymbolType *>(type)->name;
  case NodeKind::ND_ARRAY_TYPE:
    return "[]" + type_to_string(static_cast<ArrayType *>(type)->underlying);
  case NodeKind::ND_POINTER_TYPE:
    return "*" + type_to_string(static_cast<PointerType *>(type)->underlying);
  default:
    return "Unknown type";
  }
}
