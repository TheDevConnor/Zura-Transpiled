#include "type.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"

void TypeChecker::performCheck(Node::Stmt *stmt) {
  visitStmt(global_table, local_table, fn_table, stmt);

  if (!foundMain) {
    handlerError(0, 0, "No main function found",
                 "Try adding this function: \n\tconst main := fn() int { \n\t  "
                 "  return 0\n\t}",
                 "Type Error");
  }
}

void TypeChecker::handlerError(int line, int pos, std::string msg,
                               std::string note, std::string typeOfError) {
  Lexer lexer; // dummy lexer
  if (note != "")
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, true);
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, false, true);
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
