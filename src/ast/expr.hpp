#pragma once

#include <iostream>
#include <vector>

#include "ast.hpp"

class NumberExpr : public Node::Expr {
public:
    double value;

    NumberExpr(double value) : value(value) {
        kind = NodeKind::ND_NUMBER;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "NumberExpr: " << value << "\n";
    }
};

class IdentExpr : public Node::Expr {
public:
    std::string name;

    IdentExpr(std::string name) : name(name) {
        kind = NodeKind::ND_IDENT;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "IdentExpr: " << name << "\n";
    }
};

class StringExpr : public Node::Expr {
public:
    std::string value;

    StringExpr(std::string value) : value(value) {
        kind = NodeKind::ND_STRING;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "StringExpr: " << value << "\n";
    }
};

class BinaryExpr : public Node::Expr {
public:
    Node::Expr *lhs;
    Node::Expr *rhs;
    std::string op;

    BinaryExpr(Node::Expr *lhs, Node::Expr *rhs, std::string op) : lhs(lhs), rhs(rhs), op(op) {
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
    Node::Expr *expr;
    std::string op;

    UnaryExpr(Node::Expr *expr, std::string op) : expr(expr), op(op) {
        kind = NodeKind::ND_UNARY;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "UnaryExpr: \n";
        Node::printIndent(ident + 1);
        std::cout << op << "\n";
        expr->debug(ident + 1);
    }

    ~UnaryExpr() {
        delete expr;
    }
};

class GroupExpr : public Node::Expr {
public:
    Node::Expr *expr;

    GroupExpr(Node::Expr *expr) : expr(expr) {
        kind = NodeKind::ND_GROUP;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "GroupExpr: \n";
        expr->debug(ident + 1);
    }

    ~GroupExpr() {
        delete expr;
    }
};

class AssignmentExpr : public Node::Expr {
public:
    Expr *assignee;
    std::string op;
    Expr *rhs;

    AssignmentExpr(Expr *assignee, std::string op, Expr *rhs) : assignee(assignee), op(op), rhs(rhs) {
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
    Node::Expr *callee;
    std::vector<Node::Expr *> args;

    CallExpr(Node::Expr *callee, std::vector<Node::Expr *> args) : callee(callee), args(args) {
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
    Node::Expr *condition;
    Node::Expr *lhs;
    Node::Expr *rhs;

    TernaryExpr(Node::Expr *condition, Node::Expr *lhs, Node::Expr *rhs) : condition(condition), lhs(lhs), rhs(rhs) {
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
