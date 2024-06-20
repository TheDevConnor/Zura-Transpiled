#pragma once
#ifdef csk

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
        for (auto s : stmt) {
            s->debug();
        }
    }

    ~ProgramStmt() {
        for (auto s : stmt) {
            delete s;
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
        std::cout << "ExprStmt: \n";
        expr->debug();
    }

    ~ExprStmt() {
        delete expr;
    }
};

class VarStmt : public Node::Stmt {
public:
    std::string name;
    std::string type;
    ExprStmt *expr;

    VarStmt(std::string name, std::string type, ExprStmt *expr) : name(name), type(type), expr(expr) {
        kind = NodeKind::ND_VAR_STMT;
    }

    void debug() const override {
        std::cout << "VarStmt: \n\t" 
                  << "Name: " << name << "\n\t" 
                  << "Type: " << type << "\n\t"
                  << "Expr: ";
        expr->debug();
    } 

    ~VarStmt() {
        delete expr;
    }
};
#endif