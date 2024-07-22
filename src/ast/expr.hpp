#pragma once

#include <iostream>
#include <vector>

#include "ast.hpp"

class NumberExpr : public Node::Expr {
public:
  int line, pos;
  double value;

  NumberExpr(int line, int pos, double value)
      : line(line), pos(pos), value(value) {
    kind = NodeKind::ND_NUMBER;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "NumberExpr: " << value << "\n";
  }
};

class IdentExpr : public Node::Expr {
public:
  int line, pos;
  std::string name;

  IdentExpr(int line, int pos, std::string name)
      : line(line), pos(pos), name(name) {
    kind = NodeKind::ND_IDENT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "IdentExpr: " << name << "\n";
  }
};

class StringExpr : public Node::Expr {
public:
  int line, pos;
  std::string value;

  StringExpr(int line, int pos, std::string value)
      : line(line), pos(pos), value(value) {
    kind = NodeKind::ND_STRING;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "StringExpr: " << value << "\n";
  }
};

class BinaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *lhs;
  Node::Expr *rhs;
  std::string op;

  BinaryExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs,
             std::string op)
      : line(line), pos(pos), lhs(lhs), rhs(rhs), op(op) {
    kind = NodeKind::ND_BINARY;
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

  UnaryExpr(int line, int pos, Node::Expr *expr, std::string op)
      : line(line), pos(pos), expr(expr), op(op) {
    kind = NodeKind::ND_UNARY;
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

  PrefixExpr(int line, int pos, Node::Expr *expr, std::string op)
      : line(line), pos(pos), expr(expr), op(op) {
    kind = NodeKind::ND_PREFIX;
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

class GroupExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *expr;

  GroupExpr(int line, int pos, Node::Expr *expr)
      : line(line), pos(pos), expr(expr) {
    kind = NodeKind::ND_GROUP;
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
  std::vector<Node::Expr *> elements;

  ArrayExpr(int line, int pos, std::vector<Node::Expr *> elements)
      : line(line), pos(pos), elements(elements) {
    kind = NodeKind::ND_ARRAY;
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

class AssignmentExpr : public Node::Expr {
public:
  int line, pos;
  Expr *assignee;
  std::string op;
  Expr *rhs;

  AssignmentExpr(int line, int pos, Expr *assignee, std::string op, Expr *rhs)
      : line(line), pos(pos), assignee(assignee), op(op), rhs(rhs) {
    kind = NodeKind::ND_ASSIGN;
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
           std::vector<Node::Expr *> args)
      : line(line), pos(pos), callee(callee), args(args) {
    kind = NodeKind::ND_CALL;
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

class TernaryExpr : public Node::Expr {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Expr *lhs;
  Node::Expr *rhs;

  TernaryExpr(int line, int pos, Node::Expr *condition, Node::Expr *lhs,
              Node::Expr *rhs)
      : line(line), pos(pos), condition(condition), lhs(lhs), rhs(rhs) {
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

  MemberExpr(int line, int pos, Node::Expr *lhs, Node::Expr *rhs)
      : line(line), pos(pos), lhs(lhs), rhs(rhs) {
    kind = NodeKind::ND_MEMBER;
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