#pragma once

#include <iostream>
#include <vector>

#include "ast.hpp"
#include "types.hpp"

class IntExpr : public Node::Expr {
public:
  int line, pos;
  long long value;

  IntExpr(int line, int pos, long long value, int file) : line(line), pos(pos), value(value) {
    kind =  NodeKind::ND_INT;
    file_id = file;
    // make a new type of "int"
    this->asmType = new SymbolType("int");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "IntExpr: " << value << "\n";
  }
};

class FloatExpr : public Node::Expr {
public:
  int line, pos;
  float value;

  FloatExpr(int line, int pos, float value, int file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_FLOAT;
    this->asmType = new SymbolType("float");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "FloatExpr: " << value << "\n";
  }
};

class IdentExpr : public Node::Expr {
public:
  int line, pos;
  std::string name;
  Node::Type *type;

  IdentExpr(int line, int pos, std::string name, Node::Type *type, int file)
      : line(line), pos(pos), name(name), type(type) {
    kind = NodeKind::ND_IDENT;
    file_id = file;
    // Let type be redefined in typecheck (shhh)
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "IdentExpr: " << name << "\n";
    if (type != nullptr) {
      Node::printIndent(ident + 1);
      std::cout << "Type: ";
      type->debug(ident + 1);
    }
  }
};

class StringExpr : public Node::Expr {
public:
  int line, pos;
  std::string value;

  StringExpr(int line, int pos, std::string value, int file)
      : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_STRING;
    this->asmType = new SymbolType("str");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "StringExpr: " << value << "\n";
  }
};

class CastExpr : public Node::Expr {
public: 
  int line, pos;
  Node::Expr *castee;
  Node::Type *castee_type;

  CastExpr(int line, int pos, Node::Expr *castee, Node::Type *castee_type, int file)
      : line(line), pos(pos), castee(castee), castee_type(castee_type) {
    file_id = file;
    kind = NodeKind::ND_CAST;
    this->asmType = castee_type;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CastExpr: \n";
    castee->debug(indent + 1);
    Node::printIndent(indent + 1);
    std::cout << "Type: ";
    castee_type->debug(indent + 1);
    std::cout << std::endl;
  }

  ~CastExpr() {
    delete castee;
    delete castee_type;
  }
};

class BinaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;
  std::string op;

  BinaryExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
             std::string op, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs), op(op) {
    kind = NodeKind::ND_BINARY;
    file_id = file;
    // Assigning ASMType should be typechecker's problem, where it is supposed to be
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "BinaryExpr: \n";
    lhs->debug(ident + 1);
    Node::printIndent(ident + 1);
    std::cout << op << "\n";
    rhs->debug(ident + 1);
  }

  ~BinaryExpr() {
    delete lhs;
    delete rhs;
  }
};

class UnaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  UnaryExpr(int line, int pos, Node::Expr *expr, std::string op, int file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_UNARY;
    SymbolType *exprType = static_cast<SymbolType *>(expr->asmType);
    if (op == "-" && exprType->name != "int" && exprType->name != "float") {
      std::cerr << "excuse me bruh how do i negate a '" << exprType->name << "'?" << std::endl; 
    }
    asmType = exprType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "UnaryExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << op << "\n";
    expr->debug(ident + 1);
  }

  ~UnaryExpr() { delete expr; }
};
class PrefixExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  PrefixExpr(int line, int pos, Node::Expr *expr, std::string op, int file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_PREFIX;
    asmType = expr->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "PrefixExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << op << "\n";
    expr->debug(ident + 1);
  }

  ~PrefixExpr() { delete expr; }
};
class PostfixExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;
  std::string op;

  PostfixExpr(int line, int pos, Node::Expr *expr, std::string op, int file)
      : line(line), pos(pos), expr(expr), op(op) {
    file_id = file;
    kind = NodeKind::ND_POSTFIX;
    asmType = expr->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "PostfixExpr: \n";
    expr->debug(ident + 1);
    Node::printIndent(ident + 1);
    std::cout << op << "\n";
  }

  ~PostfixExpr() { delete expr; }
};

class GroupExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;

  GroupExpr(int line, int pos, Node::Expr *expr, int file)
      : line(line), pos(pos), expr(expr) {
    file_id = file;
    kind = NodeKind::ND_GROUP;
    asmType = expr->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "GroupExpr: \n";
    expr->debug(ident + 1);
  }

  ~GroupExpr() { delete expr; }
};

class ArrayExpr : public Node::Expr {
public:
  int line, pos;
  Node::Type *type;
  std::vector<Node::Expr *> elements;

  ArrayExpr(int line, int pos, Node::Type *type,
            std::vector<Node::Expr *> elements, int file)
      : line(line), pos(pos), type(type), elements(std::move(elements)) {
    file_id = file;
    kind = NodeKind::ND_ARRAY;
    asmType = type;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ArrayExpr: \n";
    for (auto elem : elements) {
      elem->debug(ident + 1);
    }
  }

  ~ArrayExpr() {
    for (auto elem : elements) {
      delete elem;
    }
  }
};

class IndexExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  IndexExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_INDEX;
    asmType = lhs->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "IndexExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~IndexExpr() {
    delete lhs;
    delete rhs;
  }
};

class PopExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  PopExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_POP;
    asmType = lhs->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "PopExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    if (rhs == nullptr) {
      std::cout << "Pop the last element\n";
      return;
    }
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~PopExpr() {
    delete lhs;
    delete rhs;
  }
};

class PushExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;
  Node::Expr *index;

  PushExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
           Node::Expr *index, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs), index(index) {
    kind = NodeKind::ND_PUSH;
    asmType = lhs->asmType;
    file_id = file;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "PushExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
    if (index != nullptr) {
      Node::printIndent(ident + 1);
      std::cout << "Index: \n";
      index->debug(ident + 2);
    }
  }

  ~PushExpr() {
    delete lhs;
    delete rhs;
  }
};

class AssignmentExpr : public Node::Expr {
public:
  int line, pos;
  Expr *assignee;
  std::string op;
  Expr *rhs;

  AssignmentExpr(int line, int pos, Expr *assignee, std::string op, Expr *rhs, int file)
      : line(line), pos(pos), assignee(assignee), op(op), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_ASSIGN;
    asmType = rhs->asmType;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "AssignmentExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "Assignee: \n";
    assignee->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Operator: " << op << "\n";
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~AssignmentExpr() {
    delete assignee;
    delete rhs;
  }
};

class CallExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *callee;
  std::vector<Node::Expr *> args;

  CallExpr(int line, int pos, Node::Expr *callee,
           std::vector<Node::Expr *> args, int file)
      : line(line), pos(pos), callee(callee), args(std::move(args)) {
    file_id = file;
    kind = NodeKind::ND_CALL;
    // Let typecheck fill out the function return
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "CallExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "Callee: \n";
    callee->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Arguments: \n";
    for (auto arg : args) {
      arg->debug(ident + 2);
    }
  }

  ~CallExpr() {
    delete callee;
    for (auto arg : args) {
      delete arg;
    }
  }
};

class TemplateCallExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *callee;
  Node::Type *template_type;
  Node::Expr *args;

  TemplateCallExpr(int line, int pos, Node::Expr *callee, Node::Type *template_type,
                   Node::Expr *args, int file)
      : line(line), pos(pos), callee(callee), template_type(template_type), args(args) {
    file_id = file;
    kind = NodeKind::ND_TEMPLATE_CALL;
    // what the fuck
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "TemplateCallExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "Callee: \n";
    callee->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "Template Type: ";
    template_type->debug(ident + 2);
    std::cout << std::endl;
    Node::printIndent(ident + 1);
    std::cout << "Arguments: \n";
    args->debug(ident + 2);
  }

  ~TemplateCallExpr() {
    delete callee;
    delete template_type;
    delete args;
  }
};

class TernaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Expr *lhs;
  Node::Expr *rhs;

  TernaryExpr(int line, int pos, Node::Expr *condition, Node::Expr *lhs,
              Node::Expr *rhs, int file)
      : line(line), pos(pos), condition(condition), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_TERNARY;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "TernaryExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "Condition: \n";
    condition->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~TernaryExpr() {
    delete condition;
    delete lhs;
    delete rhs;
  }
};

class MemberExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  MemberExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_MEMBER;
    // type check whatever the rhs is supposed to be
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "MemberExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~MemberExpr() {
    delete lhs;
    delete rhs;
  }
};

class ResolutionExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;

  ResolutionExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs, int file)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    file_id = file;
    kind = NodeKind::ND_RESOLUTION;
    std::cerr << "excuse me lets like not do the .. thing ok?" << std::endl;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ResolutionExpr: \n";
    Node::printIndent(ident + 1);
    std::cout << "LHS: \n";
    lhs->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "RHS: \n";
    rhs->debug(ident + 2);
  }

  ~ResolutionExpr() {
    delete lhs;
    delete rhs;
  }
};

class BoolExpr : public Node::Expr {
public:
  int line, pos;
  bool value;

  BoolExpr(int line, int pos, bool value, int file) : line(line), pos(pos), value(value) {
    file_id = file;
    kind = NodeKind::ND_BOOL;
    asmType = new SymbolType("bool");
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "BoolStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Value: " << value << "\n";
  }
};
