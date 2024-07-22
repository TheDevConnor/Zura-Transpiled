#include "type.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"

void TypeChecker::check(Node::Stmt *stmt) {
  symbol_table table;
  callables_table ctable;
  visitStmt(ctable, table, stmt);
}

void TypeChecker::handlerError(int line, int pos, std::string msg, std::string note) {
  Lexer lexer; // dummy lexer
  if (note != "")
    ErrorClass::error(line, pos, msg, note, "Type Error", "main.zu", lexer,
                    node.tks, 
                    false, false, true, false, true);
  ErrorClass::error(line, pos, msg, "", "Type Error", "main.zu", lexer,
                    node.tks, 
                    false, false, true, false, true);
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

void TypeChecker::check_for_main(callables_table &ctables, symbol_table &table, Node::Stmt *stmt) {
  if (stmt->kind == NodeKind::ND_FN_STMT) {
    auto fn_stmt = static_cast<fnStmt *>(stmt);
    if (fn_stmt->name == "main") {
      // Check to make sure the main function has no parameters
      if (fn_stmt->params.size() != 0) {
        handlerError(fn_stmt->line, fn_stmt->pos,
                      "Main function should not have any parameters", "");
      }
      // Check to make sure the main function has a return type of int
      if (fn_stmt->returnType->kind != NodeKind::ND_SYMBOL_TYPE) {
        handlerError(fn_stmt->line, fn_stmt->pos,
                      "Main function should have a return type of int", "");
      }
    } else {
      handlerError(fn_stmt->line, fn_stmt->pos,
                    "Main function not found in the program", "Try adding this:\n\t const main := fn() int { // your code here }");
    } 
  }
}