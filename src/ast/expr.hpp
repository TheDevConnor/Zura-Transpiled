#pragma once

#include <iostream>

#include "ast.hpp"

class NumberExpr : public Node::Expr {
public:
    std::string value;

    NumberExpr(std::string value) : value(value) {
        kind = NodeKind::ND_NUMBER;
    }

    void debug() const override {
        std::cout << value; 
    }
};

class IdentExpr : public Node::Expr {
public:
    std::string name;

    IdentExpr(std::string name) : name(name) {
        kind = NodeKind::ND_IDENT;
    }

    void debug() const override {
        std::cout << name; 
    }
};

class StringExpr : public Node::Expr {
public:
    std::string value;

    StringExpr(std::string value) : value(value) {
        kind = NodeKind::ND_STRING;
    }

    void debug() const override {
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

    void debug() const override {
        lhs->debug();
        std::cout << op << " ";
        rhs->debug();
    }
};

class UnaryExpr : public Node::Expr {
public:
    Node::Expr *expr;
    std::string op;

    UnaryExpr(Node::Expr *expr, std::string op) : expr(expr), op(op) {
        kind = NodeKind::ND_UNARY;
    }

    void debug() const override {
        std::cout << op;
        expr->debug();
    }
};

class GroupExpr : public Node::Expr {
public:
    Node::Expr *expr;

    GroupExpr(Node::Expr *expr) : expr(expr) {
        kind = NodeKind::ND_GROUP;
    }

    void debug() const override {
        std::cout << "(";
        expr->debug();
        std::cout << ")";
    }
};