#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "type.hpp"
#include <memory>

void TypeChecker::handlerError(int line, int pos, std::string msg,
                               std::string note, std::string typeOfError) {
  Lexer lexer; // dummy lexer
  if (note != "")
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, true, false);
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, false, true, false);
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

std::shared_ptr<SymbolType> TypeChecker::checkReturnType(Node::Expr *expr, const std::string &defaultType) {
    if (return_type == nullptr) {
        expr->asmType = return_type.get();
        return std::make_shared<SymbolType>(defaultType);
    }
    return std::static_pointer_cast<SymbolType>(return_type);
}

bool TypeChecker::checkTypeMatch(const std::shared_ptr<SymbolType> &lhs,
                                 const std::shared_ptr<SymbolType> &rhs,
                                 const std::string &operation, int line,
                                 int pos) {
  if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
    std::string msg =
        "Operation '" + operation + "' requires both sides to be the same type";
    handlerError(line, pos, msg, "", "Type Error");
    return false;
  }
  return true;
}
