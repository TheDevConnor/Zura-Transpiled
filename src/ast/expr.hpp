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
        Node::printIdent(ident);
        std::cout << value;
    }
};

class IdentExpr : public Node::Expr {
public:
    std::string name;

    IdentExpr(std::string name) : name(name) {
        kind = NodeKind::ND_IDENT;
    }

    void debug(int ident = 0) const override {
        Node::printIdent(ident);
        std::cout << name;
    }
};

class StringExpr : public Node::Expr {
public:
    std::string value;

    StringExpr(std::string value) : value(value) {
        kind = NodeKind::ND_STRING;
    }

    void debug(int ident = 0) const override {
        Node::printIdent(ident);
        std::cout << value;
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
       std::cout << "(";
        lhs->debug(0);
        std::cout << " " << op << " ";
        rhs->debug(0);
        std::cout << ")"; 
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
        Node::printIdent(ident);
        std::cout << op ;
        expr->debug(ident);
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
        Node::printIdent(ident);
        std::cout << "(";
        expr->debug(ident);
        std::cout << ")";
    }

    ~GroupExpr() {
        delete expr;
    }
};

class AssignmentExpr : public Node::Expr {
public:
    Expr *assigne;
    std::string op;
    Expr *rhs;

    AssignmentExpr(Expr *assigne, std::string op, Expr *rhs) : assigne(assigne), op(op), rhs(rhs) {
        kind = NodeKind::ND_ASSIGN;
    }

    void debug(int ident = 0) const override {
        Node::printIdent(ident);
        std::cout << "AssignmentExpr: \n\t";
        std::cout << "assigne: "; assigne->debug(0);
        std::cout << "\n\t";
        std::cout << "op: " << op;
        std::cout << "\n\t";
        std::cout << "rhs: "; rhs->debug(0); 
    }

    ~AssignmentExpr() {
        delete assigne;
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
        Node::printIdent(ident);
        std::cout << "CallExpr: \n\t";
        std::cout << "callee: "; callee->debug(0);
        std::cout << "\n\t";
        std::cout << "args: \n";
        for (auto a : args) {
            a->debug(ident + 1);
            std::cout << "\n";
        }
    }

    ~CallExpr() {
        delete callee;
        for (auto a : args) {
            delete a;
        }
    }
};

// have x: num = ( 1 < 2 ) ? 1 : 2;
class TernaryExpr : public Node::Expr {
public: 
    Node::Expr *condition;
    Node::Expr *lhs;
    Node::Expr *rhs;

    TernaryExpr(Node::Expr *condition, Node::Expr *lhs, Node::Expr *rhs) : condition(condition), lhs(lhs), rhs(rhs) {
        kind = NodeKind::ND_TERNARY;
    }

    void debug(int ident = 0) const override {
        Node::printIdent(ident);
        std::cout << "TernaryExpr: \n\t";
        std::cout << "condition: "; condition->debug(0);
        std::cout << "\n\t";
        std::cout << "lhs: "; lhs->debug(0);
        std::cout << "\n\t";
        std::cout << "rhs: "; rhs->debug(0);
    }

    ~TernaryExpr() {
        delete condition;
        delete lhs;
        delete rhs;
    }
};