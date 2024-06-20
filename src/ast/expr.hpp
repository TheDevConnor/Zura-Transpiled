#pragma once

#include <iostream>

#include "ast.hpp"
#ifdef csk

class NumberExpr : public Node::Expr {
public:
    std::string value;

    NumberExpr(std::string value) : value(value) {
        kind = NodeKind::ND_NUMBER;
    }

    void debug(int ident = 0) const override {
        Node::printIdent(ident);
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
        Node::printIdent(ident);
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
        Node::printIdent(ident);
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
        Node::printIdent(ident);
        std::cout << "BinaryExpr: " << op << "\n";
        Node::printIdent(ident + 1);
        std::cout << "LHS:\n";
        lhs->debug(ident + 2);
        Node::printIdent(ident + 1);
        std::cout << "RHS:\n";
        rhs->debug(ident + 2);
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
        std::cout << "UnaryExpr: " << op << "\n";
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
         Node::printIdent(ident);
        std::cout << "GroupExpr:\n";
        expr->debug(ident + 1);
    }

    ~GroupExpr() {
        delete expr;
    }
};
#endif