#pragma once

#include <iostream>
#include <vector>

#include "ast.hpp"

class ProgramStmt : public Node::Stmt {
public:
    std::vector<Node::Stmt *> stmt;

    ProgramStmt(std::vector<Node::Stmt *> stmt) : stmt(stmt) {
        kind = NodeKind::ND_PROGRAM;
    }

    void debug() const override {
        for (auto &s : stmt) {
            s->debug();
        }
    }
};

class ExprStmt : public Node::Stmt {
public:
    Node::Expr *expr;

    ExprStmt(Node::Expr *expr) : expr(expr) {
        kind = NodeKind::ND_EXPR_STMT;
    }

    void debug() const override {
        expr->debug();
    }
};

class VarStmt : public Node::Stmt {
public:
    std::string name;
    ExprStmt *expr;

    VarStmt(std::string name, ExprStmt *expr) : name(name), expr(expr) {
        kind = NodeKind::ND_VAR_STMT;
    }

    void debug() const override {
        std::cout << "var " << name << " = ";
        expr->debug();
    } 
};
