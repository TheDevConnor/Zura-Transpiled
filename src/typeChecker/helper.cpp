#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "type.hpp"
#include "typeMaps.hpp"
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

bool TypeChecker::checkTypeMatch(Node::Type *lhs, Node::Type *rhs) {
  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return false;
  }
  if (isIntBasedType(lhs) && isIntBasedType(rhs)) {
    return true;
  }
  if (lhs->kind == ND_POINTER_TYPE && rhs->kind == ND_POINTER_TYPE) {
    PointerType *l = static_cast<PointerType *>(lhs);
    PointerType *r = static_cast<PointerType *>(rhs);
    return checkTypeMatch(l->underlying, r->underlying);
  }
  if (lhs->kind == ND_ARRAY_TYPE && rhs->kind == ND_ARRAY_TYPE) {
    ArrayType *l = static_cast<ArrayType *>(lhs);
    ArrayType *r = static_cast<ArrayType *>(rhs);
    if (l->constSize != r->constSize) return false;
    return checkTypeMatch(l->underlying, r->underlying);
  }
  if (lhs->kind == ND_TEMPLATE_STRUCT_TYPE && rhs->kind == ND_TEMPLATE_STRUCT_TYPE) {
    TemplateStructType *l = static_cast<TemplateStructType *>(lhs);
    TemplateStructType *r = static_cast<TemplateStructType *>(rhs);
    if (l->name != r->name) return false;
    return checkTypeMatch(l->underlying, r->underlying);
  }
  if (lhs->kind == ND_FUNCTION_TYPE && rhs->kind == ND_FUNCTION_TYPE) {
    FunctionType *l = static_cast<FunctionType *>(lhs);
    FunctionType *r = static_cast<FunctionType *>(rhs);
    if (l->args.size() != r->args.size()) return false;
    for (size_t i = 0; i < l->args.size(); i++) {
      if (!checkTypeMatch(l->args[i], r->args[i])) return false;
    }
    return checkTypeMatch(l->ret, r->ret);
  }
  if (lhs->kind == ND_SYMBOL_TYPE && rhs->kind == ND_SYMBOL_TYPE) {
    SymbolType *l = static_cast<SymbolType *>(lhs);
    SymbolType *r = static_cast<SymbolType *>(rhs);
    // Do not check signedness ON PURPOSEEEEE
    return l->name == r->name;
  }

  throw new std::runtime_error("Not implemented checkTypeMatch helper.cpp:47");
}

std::shared_ptr<Node::Type> TypeChecker::share(Node::Type *type) {
  if (type == nullptr) return std::make_shared<SymbolType>("unknown");
  switch (type->kind) {
  case NodeKind::ND_SYMBOL_TYPE: {
    SymbolType *st = static_cast<SymbolType *>(type);
    return std::make_shared<SymbolType>(st->name, st->signedness);
  }
  case NodeKind::ND_ARRAY_TYPE: {
    ArrayType *at = static_cast<ArrayType *>(type);
    return std::make_shared<ArrayType>(at->underlying, at->constSize);
  }
  case NodeKind::ND_POINTER_TYPE:
    return std::make_shared<PointerType>(static_cast<PointerType *>(type)->underlying);
  case NodeKind::ND_TEMPLATE_STRUCT_TYPE: {
    TemplateStructType *temp = static_cast<TemplateStructType *>(type);
    return std::make_shared<TemplateStructType>(temp->name, temp->underlying);
  }
  case NodeKind::ND_FUNCTION_TYPE: {
    FunctionType *fn = static_cast<FunctionType *>(type);
    return std::make_shared<FunctionType>(fn->args, fn->ret);
  }
  default:
    handleError(0, 0, "Unknown type for share", "", "Type Error");
    return std::make_shared<SymbolType>("unknown");
  }
  
}

std::string TypeChecker::determineTypeKind(const std::string &type) {
  std::string real = type;
  if (type.find('*') == 0) {
    real = type.substr(1);
  }
  if (context->structTable.find(real) != context->structTable.end()) {
    return "struct";
  }
  if (context->enumTable.find(real) != context->enumTable.end()) {
      return "enum";
  }
  return "unknown";
}

void TypeChecker::processStructMember(MemberExpr *member, const std::string &name, std::string lhsType) {
  std::string realType = lhsType;
  if (lhsType.find('*') == 0) {
    realType = lhsType.substr(1); // We know the type directly under this is a struct (otherwise we wouldnt be here)
  }
  if (!context->structTable.contains(realType)) {
    // if we got this far, it probably exists, but its worth checking for anyway
    std::string msg = "Type '" + lhsType + "' does not have members";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }
  for (auto it : context->structTable.at(realType)) {
    if (it.first == name) {
      return_type = std::make_shared<SymbolType>(type_to_string(it.second.first));
      member->asmType = createDuplicate(return_type.get());
      return;
    }
  }
}

void TypeChecker::processEnumMember(MemberExpr *member, const std::string &lhsType) {
    std::string realType = static_cast<IdentExpr *>(member->lhs)->name;

    auto fields = context->enumTable.find(realType)->second;
    auto res = fields.find(static_cast<IdentExpr *>(member->rhs)->name);

    if (res == fields.end()) {
        std::string msg = "Type '" + realType + "' does not have member '" + static_cast<IdentExpr *>(member->rhs)->name + "'";
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

bool TypeChecker::validateArgumentCount(CallExpr *call, Node::Expr *callee, const std::unordered_map<std::string, Node::Type *> &fnParams) {
  // Check if the call is a member expr call or a call on a function from a struct
  bool isFromStruct = callee->kind == ND_MEMBER;
  size_t requiredSize = fnParams.size();
  if (isFromStruct) {
    // Subtract 1 from the size of the fnParams because the first param is the 'self' proprty
    requiredSize--;
  }

  if (call->args.size() != requiredSize) {
    std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
    std::string msg = "Function '" + functionName + "' requires " + std::to_string(fnParams.size()) + " arguments but got " + std::to_string(call->args.size());
    handleError(call->line, call->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return false;
  }
  return true;
}

bool TypeChecker::validateArgumentTypes(CallExpr *call, Node::Expr *callee, const std::unordered_map<std::string, Node::Type *> &fnParams) {
  for (size_t i = 0; i < call->args.size(); ++i) {
    visitExpr(call->args[i]);
    std::string argTypeStr = type_to_string(return_type.get());
    Node::Type *expectedType = fnParams.at(fnParams.begin()->first);
    std::string expectedTypeStr = type_to_string(expectedType);

    if (argTypeStr != expectedTypeStr) {
      // If int is compared to an unsigned int or something, let it go
      if (isIntBasedType(return_type.get()) && isIntBasedType(expectedType)) continue; // They can effectively be casted to one another
      std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
      std::string msg = "Function '" + functionName + "' requires argument '" + fnParams.begin()->first + "' to be of type '" + expectedTypeStr + "' but got '" + argTypeStr + "'";
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
    "int", "char", "short", "long"
  };
  // Do not check for signedness ON PURPOSE
  if (type->kind != ND_SYMBOL_TYPE) return false;
  SymbolType *sym = static_cast<SymbolType *>(type);
  return intTypes.find(sym->name) != intTypes.end();
}