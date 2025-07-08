#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "types.hpp"

class IntExpr : public Node::Expr {
public:
  int line, pos;
  long long value;
  bool isUnsigned = true; // this HAS to be true, because the only time an
                          // intExpr would be negative would be unaryexpr's -

  IntExpr(int line, int pos, long long value, size_t file)
      : line(line), pos(pos), value(value) {
    kind = NodeKind::ND_INT;
    file_id = file;
    // make a new type of "int"
    this->asmType = new SymbolType("int");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "IntExpr: " << value << "\n";
  }
};

class FloatExpr : public Node::Expr {
public:
  int line, pos;
  std::string value; // The reason we are using the string and not the float
                     // converston is because the user may want more precision
                     // that could be cut off later in the stof function. "Long
                     // double" literals exist, and we cannot risk the loss of
                     // precision (otherwise, why use a long double at all?)

  FloatExpr(int line, int pos, std::string value, size_t file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_FLOAT;
    this->asmType = new SymbolType("float");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "FloatExpr: " << value << "\n";
  }
};

class ExternalCall : public Node::Expr {
public:
  int line, pos;
  std::string name;
  std::vector<Node::Expr *> args;

  // Technically, these can return things. However, we can't know them
  // because they are, of course, external.
  // Their return types should be defined in public documentation
  // and it is the user's responsibility to know what they are.

  // EX: printf returns an int, but since that's part of the Cstdlib,
  // we don't know that.

  ExternalCall(int line, int pos, std::string name,
               std::vector<Node::Expr *> args, size_t file)
      : line(line), pos(pos), name(name), args(args) {
    kind = NodeKind::ND_EXTERNAL_CALL;
    file_id = file;
    asmType = new SymbolType("unknown");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ExternalCall: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Arguments: \n";
    for (Node::Expr *arg : args) {
      arg->debug(indent + 2);
    }
  }

  ~ExternalCall() = default;
  // rule of threes bro
};

class IdentExpr : public Node::Expr {
public:
  int line, pos;
  std::string name;
  Node::Type *type;

  IdentExpr(int line, int pos, std::string name, Node::Type *type, size_t file)
      : line(line), pos(pos), name(name), type(type) {
    kind = NodeKind::ND_IDENT;
    file_id = file;
    // Let type be redefined in typecheck (shhh)
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "IdentExpr: " << name << "\n";
    if (type != nullptr) {
      Node::printIndent(indent + 1);
      std::cout << "Type: \n";
      Node::printIndent(indent + 2);
      type->debug(indent + 2);
    }
  }
};

class StringExpr : public Node::Expr {
public:
  int line, pos;
  std::string value;

  StringExpr(int line, int pos, std::string value, size_t file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_STRING;
    this->asmType = new SymbolType("str");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "StringExpr: " << value << "\n";
  }
};

class CharExpr : public Node::Expr {
public:
  int line, pos;
  char value;

  CharExpr(int line, int pos, char value, size_t file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_CHAR;
    this->asmType = new SymbolType("char");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CharExpr: " << value << "\n";
  }
};

class CastExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *castee;
  Node::Type *castee_type;

  CastExpr(int line, int pos, Node::Expr *castee, Node::Type *castee_type,
           size_t file)
      : line(line), pos(pos), castee(castee), castee_type(castee_type) {
    file_id = file;
    kind = NodeKind::ND_CAST;
    this->asmType = castee_type;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CastExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "From: \n";
    castee->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "To: \n";
    Node::printIndent(
        indent +
        2); // for some reason this does not happen from within the type
    castee_type->debug(indent + 2);
  }

  ~CastExpr() = default; // rule of threes
};

class BinaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;
  std::string op;

  BinaryExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
             std::string op, size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs), op(op) {
    kind = NodeKind::ND_BINARY;
    file_id = file;
    // Assigning ASMType should be typechecker's problem, where it is supposed
    // to be
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BinaryExpr: \n";
    lhs->debug(indent + 1);
    Node::printIndent(indent + 1);
    std::cout << op << "\n";
    rhs->debug(indent + 1);
  }

  ~BinaryExpr() = default; // rule of threes
};

class UnaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  UnaryExpr(int line, int pos, Node::Expr *expr, std::string op, size_t file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_UNARY;
    SymbolType *exprType = static_cast<SymbolType *>(expr->asmType);
    if (op == "-" && exprType->name != "int" && exprType->name != "float") {
      std::cerr << "excuse me bruh how do i negate a '" << exprType->name
                << "'?" << std::endl;
    }
    asmType = exprType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "UnaryExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << op << "\n";
    expr->debug(indent + 1);
  }

  ~UnaryExpr() = default; // rule of threes
};
class PrefixExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  PrefixExpr(int line, int pos, Node::Expr *expr, std::string op, size_t file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_PREFIX;
    asmType = expr->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PrefixExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << op << "\n";
    expr->debug(indent + 1);
  }

  ~PrefixExpr() = default; // rule of threes
};
class PostfixExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  PostfixExpr(int line, int pos, Node::Expr *expr, std::string op, size_t file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_POSTFIX;
    asmType = expr->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PostfixExpr: \n";
    expr->debug(indent + 1);
    Node::printIndent(indent + 1);
    std::cout << op << "\n";
  }

  ~PostfixExpr() = default; // rule of threes
};

class GroupExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;

  GroupExpr(int line, int pos, Node::Expr *expr, size_t file)
      : line(line), pos(pos), expr(expr) {
    file_id = file;
    kind = NodeKind::ND_GROUP;
    asmType = expr->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "GroupExpr: \n";
    expr->debug(indent + 1);
  }

  ~GroupExpr() = default; // rule of threes
};

class NullExpr : public Node::Expr {
public:
  int line, pos;

  NullExpr(int line, int pos, size_t file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_NULL;
    asmType = new PointerType(new SymbolType("void"));
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "NullExpr\n";
  }

  ~NullExpr() = default; // rule of threes
};

class AddressExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *right;

  AddressExpr(int line, int pos, Node::Expr *right, size_t file)
      : line(line), pos(pos), right(right) {
    file_id = file;
    kind = NodeKind::ND_ADDRESS;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "AddressExpr: \n";
    right->debug(indent + 1);
  }

  ~AddressExpr() = default; // rule of threes
};

class DereferenceExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *left;

  DereferenceExpr(int line, int pos, Node::Expr *left, size_t file)
      : line(line), pos(pos), left(left) {
    file_id = file;
    kind = NodeKind::ND_DEREFERENCE;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "DereferenceExpr: \n";
    left->debug(indent + 1);
  }

  ~DereferenceExpr() = default; // rule of threes
};

class ArrayExpr : public Node::Expr {
public:
  int line, pos;
  Node::Type *type;
  std::vector<Node::Expr *> elements;

  ArrayExpr(int line, int pos, Node::Type *type,
            std::vector<Node::Expr *> elements, size_t file)
      : line(line), pos(pos), type(type), elements(std::move(elements)) {
    file_id = file;
    kind = NodeKind::ND_ARRAY;
    asmType = type;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ArrayExpr: \n";
    for (Node::Expr *elem : elements) {
      elem->debug(indent + 1);
    }
  }

  ~ArrayExpr() = default; // rule of threes
};

class IndexExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  IndexExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_INDEX;
    asmType = lhs->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "IndexExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~IndexExpr() = default; // rule of threes
};

// have x: [4]int = [0]; # Auto fills the array with bytes of 0.
class ArrayAutoFill : public Node::Expr {
public:
  int line, pos;
  size_t fillCount;
  Node::Type *fillType;

  ArrayAutoFill(int line, int pos, size_t file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_ARRAY_AUTO_FILL;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ArrayAutoFill: \n";
    Node::printIndent(indent + 1);
    fillType->debug(indent + 2);
  }

  ~ArrayAutoFill() = default; // rule of threes
};

class PopExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  PopExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_POP;
    asmType = lhs->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PopExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    if (rhs == nullptr) {
      std::cout << "Pop the last element\n";
      return;
    }
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~PopExpr() = default; // rule of threes
};

class PushExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;
  Node::Expr *index;

  PushExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
           Node::Expr *index, size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs), index(index) {
    kind = NodeKind::ND_PUSH;
    asmType = lhs->asmType;
    file_id = file;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PushExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
    if (index != nullptr) {
      Node::printIndent(indent + 1);
      std::cout << "Index: \n";
      index->debug(indent + 2);
    }
  }

  ~PushExpr() = default; // rule of threes
};

class AssignmentExpr : public Node::Expr {
public:
  int line, pos;
  Expr *assignee;
  std::string op;
  Expr *rhs;

  AssignmentExpr(int line, int pos, Expr *assignee, std::string op, Expr *rhs,
                 size_t file)
      : line(line), pos(pos), assignee(assignee), op(op), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_ASSIGN;
    asmType = rhs->asmType;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "AssignmentExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Assignee: \n";
    assignee->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Operator: " << op << "\n";
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~AssignmentExpr() = default; // rule of threes
};

class CallExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *callee;
  std::vector<Node::Expr *> args;

  CallExpr(int line, int pos, Node::Expr *callee,
           std::vector<Node::Expr *> args, size_t file)
      : line(line), pos(pos), callee(callee), args(std::move(args)) {
    file_id = file;
    kind = NodeKind::ND_CALL;
    // Let typecheck fill out the function return
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CallExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Callee: \n";
    callee->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Arguments: \n";
    for (Node::Expr *arg : args) {
      arg->debug(indent + 2);
    }
  }

  ~CallExpr() = default; // rule of threes
};

class TemplateCallExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *callee;
  Node::Type *template_type;
  Node::Expr *args;

  TemplateCallExpr(int line, int pos, Node::Expr *callee,
                   Node::Type *template_type, Node::Expr *args, size_t file)
      : line(line), pos(pos), callee(callee), template_type(template_type),
        args(args) {
    file_id = file;
    kind = NodeKind::ND_TEMPLATE_CALL;
    // what the fuck
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "TemplateCallExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Callee: \n";
    callee->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Template Type: ";
    template_type->debug(indent + 2);
    std::cout << std::endl;
    Node::printIndent(indent + 1);
    std::cout << "Arguments: \n";
    args->debug(indent + 2);
  }

  ~TemplateCallExpr() = default; // rule of threes
};

class TernaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Expr *lhs;
  Node::Expr *rhs;

  TernaryExpr(int line, int pos, Node::Expr *condition, Node::Expr *lhs,
              Node::Expr *rhs, size_t file)
      : line(line), pos(pos), condition(condition), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_TERNARY;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "TernaryExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Condition: \n";
    condition->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~TernaryExpr() = default; // rule of threes
};

class MemberExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  MemberExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_MEMBER;
    // type check whatever the rhs is supposed to be
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "MemberExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~MemberExpr() = default; // rule of threes
};

class ResolutionExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  ResolutionExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
                 size_t file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_RESOLUTION;
    std::cerr << "excuse me lets like not do the .. thing ok?" << std::endl;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ResolutionExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "LHS: \n";
    lhs->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "RHS: \n";
    rhs->debug(indent + 2);
  }

  ~ResolutionExpr() = default; // rule of threes
};

class BoolExpr : public Node::Expr {
public:
  int line, pos;
  bool value;

  BoolExpr(int line, int pos, bool value, size_t file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_BOOL;
    asmType = new SymbolType("bool");
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BoolStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Value: " << value << "\n";
  }
};

// { name: value, name: value, ... }
class StructExpr : public Node::Expr {
public:
  int line, pos;
  std::unordered_map<IdentExpr *, Node::Expr *> values;

  StructExpr(int line, int pos,
             std::unordered_map<IdentExpr *, Node::Expr *> values, size_t file)
      : line(line), pos(pos), values(std::move(values)) {
    file_id = file;
    kind = NodeKind::ND_STRUCT;
    // type check whatever the rhs is supposed to be
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "StructExpr: \n";
    for (size_t i = 0; i < values.size(); i++) {
      std::pair<IdentExpr *, Node::Expr *> pair =
          *std::next(values.begin(), i); // :sob:
      Node::printIndent(indent + 1);
      std::cout << "Field #" + std::to_string(i) + ": \n";
      Node::printIndent(indent + 2);
      std::cout << "Name: \n";
      Node::printIndent(indent + 3);
      std::cout << pair.first << "\n";
      Node::printIndent(indent + 2);
      std::cout << "Value: \n";
      pair.second->debug(indent + 3);
    }
  }

  ~StructExpr() = default; // rule of threes
};

class AllocMemoryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *bytesToAlloc;

  AllocMemoryExpr(int line, int pos, Node::Expr *bytes, size_t file)
      : line(line), pos(pos), bytesToAlloc(bytes) {
    file_id = file;
    kind = NodeKind::ND_ALLOC_MEMORY;
    asmType = new PointerType(new SymbolType("void"));
  };

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "AllocExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Bytes: \n";
    bytesToAlloc->debug(indent + 2);
  }
};

class FreeMemoryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *whatToFree;
  Node::Expr *bytesToFree;

  FreeMemoryExpr(int line, int pos, Node::Expr *whatToFree,
                 Node::Expr *bytesToFree, size_t file)
      : line(line), pos(pos), whatToFree(whatToFree), bytesToFree(bytesToFree) {
    file_id = file;
    kind = NodeKind::ND_FREE_MEMORY;
    asmType = new SymbolType("int");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "FreeMemoryExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "What to free: \n";
    whatToFree->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Bytes to free: \n";
    bytesToFree->debug(ident + 2);
  }
};

class SizeOfExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *whatToSizeOf;

  SizeOfExpr(int line, int pos, Node::Expr *whatToSizeOf, size_t file)
      : line(line), pos(pos), whatToSizeOf(whatToSizeOf) {
    file_id = file;
    kind = NodeKind::ND_SIZEOF;
    asmType = new SymbolType("int");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "SizeOfExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "What to size of: \n";
    whatToSizeOf->debug(ident + 2);
  }
};

class MemcpyExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *dest;
  Node::Expr *src;
  Node::Expr *bytes;

  MemcpyExpr(int line, int pos, Node::Expr *dest, Node::Expr *src,
             Node::Expr *bytes, size_t file)
      : line(line), pos(pos), dest(dest), src(src), bytes(bytes) {
    file_id = file;
    kind = NodeKind::ND_MEMCPY_MEMORY;
    asmType = new SymbolType("int");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "Memcpy: \n";
    Node::printIndent(ident + 1);
    std::cout << "Destination: \n";
    dest->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Source: \n";
    src->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Bytes: \n";
    bytes->debug(ident + 2);
  }
};

class OpenExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *filename;
  Node::Expr *flags;
  // flags
  // TODO: Create global constants for these and allow for them to be OR'd
  // together manually
  // TODO: Most imporant flags: O_CREAT
  Node::Expr *canRead;
  Node::Expr *canWrite;
  Node::Expr *canCreate;

  OpenExpr(int line, int pos, Node::Expr *filename, Node::Expr *canRead,
           Node::Expr *canWrite, Node::Expr *canCreate, size_t file)
      : line(line), pos(pos), filename(filename), canRead(canRead),
        canWrite(canWrite), canCreate(canCreate) {
    file_id = file;
    kind = NodeKind::ND_OPEN;
    asmType = new SymbolType("int");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "OpenExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "Filename: \n";
    filename->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Flags: \n";
    flags->debug(ident + 2);
  }
};

class GetArgvExpr : public Node::Expr {
public:
  int line, pos;

  GetArgvExpr(int line, int pos, size_t file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_GETARGV;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "GetArgv Expr -> @getArgv" << std::endl;
  }
};

class GetArgcExpr : public Node::Expr {
public:
  int line, pos;

  GetArgcExpr(int line, int pos, size_t file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_GETARGC;
  }

  void debug(int indent = 0) const override {
    (void)indent;
    std::cout << "GetArgc Expr -> @getArgc" << std::endl;
  }
};

class StrCmp : public Node::Expr {
public:
  int line, pos;
  Node::Expr *v1;
  Node::Expr *v2;

  StrCmp(int line, int pos, Node::Expr *v1, Node::Expr *v2, size_t file)
      : line(line), pos(pos), v1(v1), v2(v2) {
    file_id = file;
    kind = NodeKind::ND_STRCMP;
  }

  void debug(int indent = 0) const override {
    std::cout << "Strcmp Expr:" << std::endl;
    std::cout << "   value 1: ";
    v1->debug(indent);
    std::cout << "   value 2: ";
    v2->debug(indent);
  }
};

class SocketExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *domain;     // this is the part of the internet to be listening to, like IPv4 (AF_INET)
  Node::Expr *socketType; // how to handle the data, like TCP (SOCK_STREAM) or UDP (SOCK_DGRAM)
  Node::Expr *protocol;   // each SocketType has a protocol but we can infer this with 0

  SocketExpr(int line, int pos, Node::Expr *domain, Node::Expr *socketType,
             Node::Expr *protocol, size_t file)
      : line(line), pos(pos), domain(domain), socketType(socketType),
        protocol(protocol) {
    file_id = file;
    kind = NodeKind::ND_SOCKET;
    asmType = new SymbolType("int", SymbolType::Signedness::UNSIGNED); // Returns a file descriptor or a negative error code
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "SocketExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Domain: \n";
    domain->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Socket Type: \n";
    socketType->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Protocol: \n";
    protocol->debug(indent + 2);
  }
};

class BindExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *socket;  // The socket to bind
  Node::Expr *structPtr; // A pointer to the struct containing bind information
  Node::Expr *structSize;    // The size of the address struct

  BindExpr(int line, int pos, Node::Expr *socket, Node::Expr *structPtr,
           Node::Expr *structSize, size_t file)
      : line(line), pos(pos), socket(socket), structPtr(structPtr), structSize(structSize) {
    file_id = file;
    kind = NodeKind::ND_BIND;
    asmType = new SymbolType("int", SymbolType::Signedness::SIGNED); // Returns an int status code
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BindExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Socket: \n";
    socket->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Address: \n";
    structPtr->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Port: \n";
    structSize->debug(indent + 2);
  }
};

class ListenExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *socket;  // The socket to listen on
  Node::Expr *backlog; // The maximum number of conections that can be waiting before they are refused

  ListenExpr(int line, int pos, Node::Expr *socket, Node::Expr *backlog,
             size_t file)
      : line(line), pos(pos), socket(socket), backlog(backlog) {
    file_id = file;
    kind = NodeKind::ND_LISTEN;
    asmType = new SymbolType("int"); // Returns an int status code
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ListenExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Socket: \n";
    socket->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Backlog: \n";
    backlog->debug(indent + 2);
  }
};

class AcceptExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *socketFd; // The socket to accept connections on
  Node::Expr *structPtr;   // This is the pointer to the address structure. It can be NIL.
  Node::Expr *structSize;   // This is the size of the address structure. It can be 0.

  AcceptExpr(int line, int pos, Node::Expr *socketFd, Node::Expr *structPtr,
             Node::Expr *structSize, size_t file)
      : line(line), pos(pos), socketFd(socketFd), structPtr(structPtr), structSize(structSize) {
    file_id = file;
    kind = NodeKind::ND_ACCEPT;
    asmType = new SymbolType("int", SymbolType::Signedness::SIGNED); // Returns a socket descriptor
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "AcceptExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Socket: \n";
    socketFd->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Struct Pointer: \n";
    structPtr->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Struct Size: \n";
    structSize->debug(indent + 2);
  }
};

class RecvExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *socketFd; // The socket to receive data from
  Node::Expr *buffer;   // The buffer to store the received data
  Node::Expr *length;   // The length of the buffer
  Node::Expr *flags;    // Flags for the receive operation (optional)

  RecvExpr(int line, int pos, Node::Expr *socketFd, Node::Expr *buffer,
           Node::Expr *length, Node::Expr *flags, size_t file)
      : line(line), pos(pos), socketFd(socketFd), buffer(buffer), length(length), flags(flags) {
    file_id = file;
    kind = NodeKind::ND_RECV;
    asmType = new SymbolType("int", SymbolType::Signedness::SIGNED); // Returns the number of bytes received
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "RecvExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Socket: \n";
    socketFd->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Buffer: \n";
    buffer->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Length: \n";
    length->debug(indent + 2);
  }
};

class SendExpr : public Node::Expr {
public:
  int line, pos;
  // Under the hood, this syscall is a SENDTO. It actually has 6 arguments,
  // but the last 2 can be NULL and 0.
  Node::Expr *socketFd; // The socket to send data through
  Node::Expr *buffer;   // The buffer containing the data to send
  Node::Expr *length;   // The length of the data to send
  Node::Expr *flags;    // Flags for the send operation (optional)

  SendExpr(int line, int pos, Node::Expr *socketFd, Node::Expr *buffer,
           Node::Expr *length, Node::Expr *flags, size_t file)
      : line(line), pos(pos), socketFd(socketFd), buffer(buffer), length(length), flags(flags) {
    file_id = file;
    kind = NodeKind::ND_SEND;
    asmType = new SymbolType("int", SymbolType::Signedness::SIGNED); // Returns the number of bytes sent
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "SendExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Socket: \n";
    socketFd->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Buffer: \n";
    buffer->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Length: \n";
    length->debug(indent + 2);
  }
};

class CommandExpr : public Node::Expr {
public:
  int line, pos;
  std::string command;
  std::vector<Node::Expr *> args; // This is a list of commands to execute

  CommandExpr(int line, int pos, std::string command,
              std::vector<Node::Expr *> args, size_t file)
      : line(line), pos(pos), command(std::move(command)), args(std::move(args)) {
    file_id = file;
    kind = NodeKind::ND_COMMAND;
    // type check whatever the rhs is supposed to be
    asmType = new SymbolType("str"); // Returns an str 
  }
  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CommandExpr: \n";
    Node::printIndent(indent + 1);
    std::cout << "Command: " << command << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Arguments: \n";
    for (const auto &arg : args) {
      arg->debug(indent + 2); 
    }
  }
};