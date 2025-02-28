#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "type.hpp"
#include <unordered_set>
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
  if (type == nullptr)
    return "unknown";
  switch (type->kind) {
  case NodeKind::ND_SYMBOL_TYPE:
    return static_cast<SymbolType *>(type)->name;
  case NodeKind::ND_ARRAY_TYPE:
    return "[]" + type_to_string(static_cast<ArrayType *>(type)->underlying);
  case NodeKind::ND_POINTER_TYPE: 
    return "*" + type_to_string(static_cast<PointerType *>(type)->underlying);
  case NodeKind::ND_TEMPLATE_STRUCT_TYPE: {
    TemplateStructType *temp = static_cast<TemplateStructType *>(type);
    return type_to_string(temp->name);
  }
  case NodeKind::ND_FUNCTION_TYPE:
    return type_to_string(static_cast<FunctionType *>(type)->ret);
  default: // Should never happen, but Connor (aka I) wrote this terrible code so anything is possable
    if (!isLspMode) std::cout << "Nodekind: " << std::to_string((int)type->kind) << std::endl;
    handleError(0, 0, "Unknown type for type_to_string", "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return "unknown";
  }
}

std::shared_ptr<SymbolType> TypeChecker::checkReturnType(Node::Expr *expr, const std::string &defaultType) {
    if (return_type == nullptr) {
        expr->asmType = new SymbolType(defaultType);
        return std::make_shared<SymbolType>(defaultType);
    }

    // check if we have a return type of any
    if (type_to_string(return_type.get()) == "any") {
        expr->asmType = new SymbolType(defaultType);
        return std::make_shared<SymbolType>(defaultType); // return the default type
    }
    return std::static_pointer_cast<SymbolType>(return_type);
}

bool TypeChecker::checkTypeMatch(const std::shared_ptr<SymbolType> &lhs,
                                 const std::shared_ptr<SymbolType> &rhs,
                                 const std::string &operation, int line,
                                 int pos, std::string &msg) {
  if (type_to_string(lhs.get()) != type_to_string(rhs.get())) {
    // if one side is of type 'any' let the other side be the type so it is not an error
    if (type_to_string(lhs.get()) == "any" || type_to_string(rhs.get()) == "any") return true;
    // Compare any int type to the builtin int typeturn true;
    if (isIntBasedType(lhs.get()) && isIntBasedType(rhs.get())) return true;
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
  if (type == "enum") { // When enum names get looked up in the symbol table, their type is "enum".
      return "enum";
  }
  return "unknown";
}

void TypeChecker::processStructMember(Maps *map, MemberExpr *member, const std::string &name, std::string lhsType) {
  std::string realType = lhsType;
  if (lhsType.find('*') == 0) {
    realType = lhsType.substr(1);
  }

  if (map->struct_table.find(realType) != map->struct_table.end()) {
    // Check if we are looking at a function or a field
    if (map->struct_table_fn.find(realType) != map->struct_table_fn.end()) {
      const FnVector &stmts = map->struct_table_fn[realType];
      const FnVector::const_iterator res = std::find_if(stmts.begin(), stmts.end(), [&member](const Fn &fn) {
          return fn.first.first == static_cast<IdentExpr *>(member->rhs)->name;
      });
      if (res == stmts.end()) {} // This is a field
      else {
        return_type = std::make_shared<SymbolType>(type_to_string(res->first.second));
        member->asmType = createDuplicate(return_type.get());
        return;
      }
    }
    const std::vector<std::pair<std::string, Node::Type *>> &fields = map->struct_table[realType];
    const std::vector<std::pair<std::string, Node::Type *>>::const_iterator
        res = std::find_if(fields.begin(), fields.end(),[&name](const std::pair<std::string, Node::Type *> &field) {
          return field.first == name;
        });
    // this cannot be fields.end() because that is actually a valid return
    if (res == fields.end()) {
      std::string msg = "Type '" + realType + "' does not have member '" + name + "'";
      handleError(member->line, member->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return;
    }
    return_type = std::make_shared<SymbolType>(type_to_string(res->second));
    member->asmType = createDuplicate(return_type.get());
  }
}

void TypeChecker::processEnumMember(Maps *map, MemberExpr *member, const std::string &lhsType) {
    std::string realType = static_cast<IdentExpr *>(member->lhs)->name;
    const std::vector<std::pair<std::string, int>> &fields = map->enum_table[realType];
    const std::vector<std::pair<std::string, int>>::const_iterator res = std::find_if(fields.begin(), fields.end(),
                            [&member](const std::pair<std::string, int> &field) {
                                return field.first == static_cast<IdentExpr *>(member->rhs)->name;
                            });
    if (res == fields.end()) {
        std::string msg = "Type '" + realType + "' does not have member '" +
                          static_cast<IdentExpr *>(member->rhs)->name + "'";
        handleError(member->line, member->pos, msg, "", "Type Error");
        return_type = std::make_shared<SymbolType>("unknown");
        return;
    }

    return_type = std::make_shared<SymbolType>(realType);
    member->asmType = createDuplicate(return_type.get());
}

void TypeChecker::handleUnknownType(MemberExpr *member, const std::string &lhsType) {
    std::string msg = "Type '" + lhsType + "' does not have members";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
}

void TypeChecker::reportOverloadedFunctionError(CallExpr *call, Node::Expr *callee) {
  std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
  std::string msg = "Function '" + functionName + "' is overloaded";
  handleError(call->line, call->pos, msg, "", "Type Error");
  return_type = std::make_shared<SymbolType>("unknown");
}

bool TypeChecker::validateArgumentCount(CallExpr *call, Node::Expr *callee,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
  // Check if the call is a member expr call or a call on a function from a struct
  bool isFromStruct = callee->kind == ND_MEMBER;
  size_t requiredSize = fnParams.size();
  if (isFromStruct) {
    // Subtract 1 from the size of the fnParams because the first param is the 'self' proprty
    requiredSize--;
  }

  if (call->args.size() != requiredSize) {
    std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
    std::string msg = "Function '" + functionName + "' requires " +
                      std::to_string(fnParams.size()) + " arguments but got " +
                      std::to_string(call->args.size());
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return false;
  }
  return true;
}

bool TypeChecker::validateArgumentTypes(Maps *map, CallExpr *call, Node::Expr *callee,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams) {
  for (size_t i = 0; i < call->args.size(); ++i) {
    visitExpr(map, call->args[i]);
    std::string argTypeStr = type_to_string(return_type.get());
    Node::Type *expectedType = fnParams[i + (callee->kind == ND_MEMBER ? 1 : 0)].second;
    std::string expectedTypeStr = type_to_string(expectedType);

    if (argTypeStr != expectedTypeStr) {
      // If int is compared to an unsigned int or something, let it go
      if (isIntBasedType(return_type.get()) && isIntBasedType(expectedType)) continue; // They can effectively be casted to one another
      std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
      std::string msg = "Function '" + functionName + "' requires argument '" +
                        fnParams[i].first + "' to be of type '" + expectedTypeStr +
                        "' but got '" + argTypeStr + "'";
      handleError(call->line, call->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return false;
    }
  }
  return true;
}


Node::Type *TypeChecker::createDuplicate(Node::Type *type) {
  if (type == nullptr) return nullptr;
  switch(type->kind) {
    case NodeKind::ND_SYMBOL_TYPE: {
      SymbolType *sym = static_cast<SymbolType *>(type);
      return new SymbolType(sym->name);
    }
    case NodeKind::ND_ARRAY_TYPE: {
      ArrayType *arr = static_cast<ArrayType *>(type);
      return new ArrayType(createDuplicate(arr->underlying), arr->constSize);
    }
    case NodeKind::ND_POINTER_TYPE: {
      PointerType *ptr = static_cast<PointerType *>(type);
      return new PointerType(createDuplicate(ptr->underlying));
    }
    case NodeKind::ND_TEMPLATE_STRUCT_TYPE: {
      TemplateStructType *temp = static_cast<TemplateStructType *>(type);
      return new TemplateStructType(createDuplicate(temp->name), createDuplicate(temp->underlying));
    }
    default: {
      std::string msg = "Unknown type";
      TypeChecker::handleError(0, 0, msg, "", "Type Error");
      return nullptr;
    }
  }
}

bool TypeChecker::isIntBasedType(Node::Type *type) {
  const static std::unordered_set<std::string> intTypes = {
    "int", "signed int", "unsigned int", "char"
  };

  return intTypes.find(type_to_string(type)) != intTypes.end();
}