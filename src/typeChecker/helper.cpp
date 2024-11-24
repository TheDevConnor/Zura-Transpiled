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

std::string TypeChecker::determineTypeKind(Maps *map, const std::string &type) {
    if (map->struct_table.find(type) != map->struct_table.end()) {
        return "struct";
    }
    if (map->enum_table.find(type) != map->enum_table.end()) {
        return "enum";
    }
    return "unknown";
}

void TypeChecker::processStructMember(Maps *map, MemberExpr *member, const std::string &lhsType) {
    const auto &fields = map->struct_table[lhsType];
    auto res = std::find_if(fields.begin(), fields.end(),
                            [&member](const auto &field) {
                                return field.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + lhsType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handlerError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    return_type = std::make_shared<SymbolType>(type_to_string(res->second));
    member->asmType = return_type.get();
}

void TypeChecker::processEnumMember(Maps *map, MemberExpr *member, const std::string &lhsType) {
    const auto &fields = map->enum_table[lhsType];
    auto res = std::find_if(fields.begin(), fields.end(),
                            [&member](const auto &field) {
                                return field.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + lhsType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handlerError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    return_type = std::make_shared<SymbolType>("int");
    member->asmType = return_type.get();
}

void TypeChecker::handleUnknownType(MemberExpr *member, const std::string &lhsType) {
    std::string msg = "Type '" + lhsType + "' does not have members";
    handlerError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
}

void TypeChecker::reportOverloadedFunctionError(CallExpr *call, const std::string &functionName) {
    std::string msg = "Function '" + functionName + "' is overloaded";
    handlerError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
}

bool TypeChecker::validateArgumentCount(CallExpr *call, const std::string &functionName,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
    if (fnParams.size() != call->args.size()) {
        std::string msg = "Function '" + functionName + "' requires " +
                          std::to_string(fnParams.size()) + " arguments but got " +
                          std::to_string(call->args.size());
        handlerError(call->line, call->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return false;
    }
    return true;
}

bool TypeChecker::validateArgumentTypes(Maps *map, CallExpr *call, const std::string &functionName,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
    for (size_t i = 0; i < call->args.size(); ++i) {
        visitExpr(map, call->args[i]);
        auto argType = type_to_string(return_type.get());
        auto expectedType = type_to_string(fnParams[i].second);

        if (argType != expectedType) {
            std::string msg = "Function '" + functionName + "' requires argument '" +
                              fnParams[i].first + "' to be of type '" + expectedType +
                              "' but got '" + argType + "'";
            handlerError(call->line, call->pos, msg, "", "Type Error");
            return_type = std::make_shared<SymbolType>("unknown");
            return false;
        }
    }
    return true;
}