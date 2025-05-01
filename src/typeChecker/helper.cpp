#include <memory>
#include <unordered_set>

#include "../helper/error/error.hpp"
#include "type.hpp"
#include "typeMaps.hpp"

void TypeChecker::handleError(int line, int pos, std::string msg,
                              std::string note, std::string typeOfError) {
  Lexer lexer;  // dummy lexer
  if (note != "") {
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, true, false);
    return;  // don't print the error twice!
  }
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, false, true, false);
}

std::string TypeChecker::type_to_string(Node::Type *type) {
  if (type == nullptr)
    return "unknown";
  switch (type->kind) {
    case NodeKind::ND_SYMBOL_TYPE: {
      switch (static_cast<SymbolType *>(type)->signedness) {
        case SymbolType::Signedness::SIGNED:
          return static_cast<SymbolType *>(type)->name + '?';
        case SymbolType::Signedness::UNSIGNED:
          return static_cast<SymbolType *>(type)->name + '!';
        default:
        case SymbolType::Signedness::INFER:
          return static_cast<SymbolType *>(type)->name;
      };
    }
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
    default:  // Should never happen, but Connor (aka I) wrote this terrible code so anything is possable
      if (!isLspMode) std::cout << "Nodekind: " << std::to_string((int)type->kind) << std::endl;
      handleError(0, 0, "Unknown type for type_to_string", "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return "unknown";
  }
}

bool TypeChecker::checkTypeMatch(Node::Type *lhs, Node::Type *rhs) {
  if (lhs->kind == ND_SYMBOL_TYPE) {
    // check if the LHS is of type void
    SymbolType *l = static_cast<SymbolType *>(lhs);
    if (l->name == "void") return true;
    // else let it fall through
  }
  if (lhs == nullptr || rhs == nullptr) {
    return_type = std::make_shared<SymbolType>("unknown");
    return false;
  }
  if (isIntBasedType(lhs) && isIntBasedType(rhs)) {
    // Check the signedness of both sides
    SymbolType *l = static_cast<SymbolType *>(lhs);
    SymbolType *r = static_cast<SymbolType *>(rhs);
    if (l->name != r->name) return false;  // different sizes; ie int vs char or long vs short
    // now it is int vs int or stuff liek that
    if (l->signedness == r->signedness) return true;
    if (l->signedness == SymbolType::Signedness::SIGNED && r->signedness == SymbolType::Signedness::UNSIGNED) return true;  // its fine because all unsigned integers can be casted
    // Check if either side is inferred. Its automatically true
    if (l->signedness == SymbolType::Signedness::INFER || r->signedness == SymbolType::Signedness::INFER) return true;
    return false;  // assign signed to unsigned
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

  std::cerr << "helper.cpp:102" << std::endl;
  std::cerr << "Unknown checkTypeMatch! lhs: " << type_to_string(lhs) << " rhs: " << type_to_string(rhs) << std::endl;
  return false;
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
  if (type.find('*') == 0) real = type.substr(1);

  if (type == "enum") return "enum";
  if (type == "struct") return "struct";

  if (context->structTable.contains(real)) return "struct";
  if (context->enumTable.contains(real)) return "enum";

  return "unknown";
}

void TypeChecker::processStructMember(MemberExpr *member, const std::string &name, std::string lhsType) {
  std::string realType = lhsType;
  if (lhsType.find('*') == 0) {
    realType = lhsType.substr(1);  // We know the type directly under this is a struct (otherwise we wouldnt be here)
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
      return_type = share(it.second.first);
      member->asmType = createDuplicate(return_type.get());
      return;
    }
  }
}

void TypeChecker::processEnumMember(MemberExpr *member, const std::string &lhsType) {
  std::string name = static_cast<IdentExpr *>(member->lhs)->name;
  std::string field = static_cast<IdentExpr *>(member->rhs)->name;

  long long value = context->enumTable.lookup(name, field);
  if (value == -1) {
    std::string msg = "Enum '" + lhsType + "' does not have member '" + field + "'";
    handleError(member->line, member->pos, msg, "", "Type Error");
    return_type = std::make_shared<SymbolType>("unknown");
    return;
  }

  return_type = std::make_shared<SymbolType>(name);
  member->asmType = new SymbolType("enum");
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
    Node::Type *argType = return_type.get();
    Node::Type *expectedType = fnParams.at(fnParams.begin()->first);

    if (!checkTypeMatch(argType, expectedType)) {
      // If int is compared to an unsigned int or something, let it go
      if (isIntBasedType(argType) && isIntBasedType(expectedType)) continue;  // They can effectively be casted to one another
      std::string functionName = callee->kind == ND_IDENT ? static_cast<IdentExpr *>(callee)->name : static_cast<IdentExpr *>(static_cast<MemberExpr *>(callee)->lhs)->name;
      std::string msg = "Function '" + functionName + "' requires argument '" + fnParams.begin()->first + "' to be of type '" + type_to_string(expectedType) + "' but got '" + type_to_string(argType) + "'";
      handleError(call->line, call->pos, msg, "", "Type Error");
      return_type = std::make_shared<SymbolType>("unknown");
      return false;
    }
  }
  return true;
}

Node::Type *TypeChecker::createDuplicate(Node::Type *type) {
  if (type == nullptr) return nullptr;
  switch (type->kind) {
    case NodeKind::ND_SYMBOL_TYPE: {
      SymbolType *sym = static_cast<SymbolType *>(type);
      return new SymbolType(sym->name, sym->signedness);
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
      "int", "char", "short", "long", "enum", "$",  // add more, maybe even a superlong for 128-bit lmao
  };

  // Do not check for signedness ON PURPOSE
  if (type->kind != ND_SYMBOL_TYPE) return false;
  SymbolType *sym = static_cast<SymbolType *>(type);
  // enum table
  if (context->enumTable.contains(sym->name)) return true;
  return intTypes.find(sym->name) != intTypes.end();
}
