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
        expr->debug();
    }

    ~ExprStmt() {
        delete expr;
    }
};

class BlockStmt : public Node::Stmt {
public:
    std::vector<Node::Stmt *> stmts;

    BlockStmt(std::vector<Node::Stmt *> stmts) : stmts(stmts) {
        kind = NodeKind::ND_BLOCK_STMT;
    }

    void debug() const override {
        std::cout << "BlockStmt: \n";
        for (auto s : stmts) {
            s->debug();
        }
        std::cout << "\nEnd of BlockStmt\n";
    }

    ~BlockStmt() {
        for (auto s : stmts) {
            delete s;
        }
    }
};

class VarStmt : public Node::Stmt {
public:
    std::string name;
    Node::Type *type;
    ExprStmt *expr;

    VarStmt(std::string name, Node::Type *type, ExprStmt *expr) : name(name), type(type), expr(expr) {
        kind = NodeKind::ND_VAR_STMT;
    }

    void debug() const override {
        std::cout << "VarStmt: \n\t" 
                  << "Name: " << name << "\n\t" 
                  << "Type: "; type->debug(); std::cout << "\n\t"
                  << "Expr: "; expr->debug(); 
        std::cout << std::endl;
    } 

    ~VarStmt() {
        delete expr;
        delete type;
    }
};

class fnStmt : public Node::Stmt {
public:
    std::string name;
    std::vector<std::pair<std::string, Node::Type *>> params;
    Node::Type *returnType;
    Node::Stmt *block;

    fnStmt(std::string name, std::vector<std::pair<std::string, Node::Type *>> params, 
            Node::Type *returnType, Node::Stmt *block) : 
            name(name), params(params), returnType(returnType), block(block) {
        kind = NodeKind::ND_FN_STMT;
    }

    void debug() const override {
        std::cout << "fnStmt: \n\t" 
                  << "Name: " << name << "\n\t" 
                  << "Params: "; 
        for (auto p : params) {
            std::cout << "\n\t\t"
                      << "Name: " << p.first << "\n\t\t"
                      << "Type: "; p.second->debug();
        }
        std::cout << "\n\t"
                  << "ReturnType: "; returnType->debug(); std::cout << "\n\t"
                  << "Block: "; block->debug(); 
        std::cout << std::endl;
    }

    ~fnStmt() {
        delete block;
        for (auto p : params) {
            delete p.second;
        }
        delete returnType;
    }
};

class ReturnStmt : public Node::Stmt {
public:
    Node::Expr *expr;

    ReturnStmt(Node::Expr *expr) : expr(expr) {
        kind = NodeKind::ND_RETURN_STMT;
    }

    void debug() const override {
        std::cout << "ReturnStmt: \n\t"
                  << "Expr: "; expr->debug();
        std::cout << std::endl;
    }

    ~ReturnStmt() {
        delete expr;
    }
};