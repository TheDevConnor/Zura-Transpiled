#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "type.hpp"
#include <memory>

void TypeChecker::handleError(int line, int pos, std::string msg,
                               std::string note, std::string typeOfError) {
  Lexer lexer; // dummy lexer
  if (note != "") {
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, true, false);
    return; // don't print the error twice!
  }
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
    // NullExpr is valid for all pointer types
    if (expr->kind == NodeKind::ND_NULL) {
        expr->asmType = return_type.get();
        return std::make_shared<SymbolType>(defaultType);
    }
    return std::static_pointer_cast<SymbolType>(return_type);
}

bool TypeChecker::checkTypeMatch(const std::shared_ptr<SymbolType> &lhs,
                                 const std::shared_ptr<SymbolType> &rhs,
                                 const std::string &operation, int line,
                                 int pos, std::string &msg) {
  if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
    handleError(line, pos, msg, "", "Type Error");
    return false;
  }
  return true;
}

std::string TypeChecker::determineTypeKind(Maps *map, const std::string &type) {
  std::string real = type;
  if (type.find('*') == 0) {
    real = type.substr(1);
  }
  if (map->struct_table.find(real) != map->struct_table.end()) {
      return "struct";
  }
  if (map->enum_table.find(real) != map->enum_table.end()) {
      return "enum";
  }
  return "unknown";
}

void TypeChecker::processStructMember(Maps *map, MemberExpr *member, const std::string &name, std::string lhsType) {
  std::cout << "Processing struct member" << std::endl;

  std::string realType = lhsType;
  if (lhsType.find('*') == 0) {
    realType = lhsType.substr(1);
  }

  // check if we are in the struct_table_fn or struct_table
  if (map->struct_table.find(realType) != map->struct_table.end()) {
    const std::vector<std::pair<std::string, Node::Type *>> &fields = map->struct_table[realType];
    const std::vector<std::pair<std::string, Node::Type *>>::const_iterator res = std::find_if(fields.begin(), fields.end(),
                            [&member](const std::pair<std::string, Node::Type *> &field) {
                                return field.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + lhsType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handleError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    // if the member expr returns a struct,
    // append struct- to the start of the asmtype
    return_type = std::make_shared<SymbolType>(type_to_string(res->second));
    if (map->struct_table.find(type_to_string(res->second)) != map->struct_table.end()) {
      member->asmType = new SymbolType("struct-" + type_to_string(return_type.get()));
    } else {
      member->asmType = return_type.get();
    }
  } else {
    FnVector fields = map->struct_table_fn[realType];
    FnVector::const_iterator res = std::find_if(fields.begin(), fields.end(),
                            [&member](Fn field) {
                                return field.first.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + lhsType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handleError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    // if the member expr returns a struct,
    // append struct- to the start of the asmtype
    return_type = std::make_shared<SymbolType>(type_to_string(res->first.second));
    if (map->struct_table.find(type_to_string(res->first.second)) != map->struct_table.end()) {
      member->asmType = new SymbolType("struct-" + type_to_string(return_type.get()));
    } else {
      member->asmType = return_type.get();
    }
  }
}

void TypeChecker::processEnumMember(Maps *map, MemberExpr *member, const std::string &lhsType) {
    const std::vector<std::pair<std::string, int>> &fields = map->enum_table[lhsType];
    const std::vector<std::pair<std::string, int>>::const_iterator res = std::find_if(fields.begin(), fields.end(),
                            [&member](const std::pair<std::string, int> &field) {
                                return field.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + lhsType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handleError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    return_type = std::make_shared<SymbolType>(lhsType);
    member->asmType = return_type.get();
}

void TypeChecker::handleUnknownType(MemberExpr *member, const std::string &lhsType) {
    std::string msg = "Type '" + lhsType + "' does not have members";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
}

void TypeChecker::reportOverloadedFunctionError(CallExpr *call, const std::string &functionName) {
    std::string msg = "Function '" + functionName + "' is overloaded";
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
}

bool TypeChecker::validateArgumentCount(CallExpr *call, const std::string &functionName,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
    if (fnParams.size() != call->args.size()) {
        std::string msg = "Function '" + functionName + "' requires " +
                          std::to_string(fnParams.size()) + " arguments but got " +
                          std::to_string(call->args.size());
        handleError(call->line, call->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return false;
    }
    return true;
}

bool TypeChecker::validateArgumentTypes(Maps *map, CallExpr *call, const std::string &functionName,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
    for (size_t i = 0; i < call->args.size(); ++i) {
        visitExpr(map, call->args[i]);
        std::string argType = type_to_string(return_type.get());
        std::string expectedType = type_to_string(fnParams[i].second);

        if (argType != expectedType) {
            std::string msg = "Function '" + functionName + "' requires argument '" +
                              fnParams[i].first + "' to be of type '" + expectedType +
                              "' but got '" + argType + "'";
            handleError(call->line, call->pos, msg, "", "Type Error");
            return_type = std::make_shared<SymbolType>("unknown");
            return false;
        }
    }
    return true;
}