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

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "ProgramStmt: \n";
        for (auto s : stmt) {
            s->debug(ident + 1);
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

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "ExprStmt: \n";
        expr->debug(ident + 1);
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

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "BlockStmt: \n";
        for (auto s : stmts) {
            s->debug(ident + 1);
        }
        Node::printIndent(ident);
        std::cout << "End of BlockStmt\n";
    }

    ~BlockStmt() {
        for (auto s : stmts) {
            delete s;
        }
    }
};

class VarStmt : public Node::Stmt {
public:
    bool isConst;
    std::string name;
    Node::Type *type;
    ExprStmt *expr;

    VarStmt(bool isConst, std::string name, Node::Type *type, ExprStmt *expr) : isConst(isConst), name(name), type(type), expr(expr) {
        kind = NodeKind::ND_VAR_STMT;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "VarStmt: \n";
        Node::printIndent(ident + 1);
        std::cout << "IsConst: " << isConst << "\n";
        Node::printIndent(ident + 1);
        std::cout << "Name: " << name << "\n";
        Node::printIndent(ident + 1);
        std::cout << "Type: ";
        type->debug(ident + 2);
        std::cout << "\n";
        Node::printIndent(ident + 1);
        std::cout << "Expr: \n";
        expr->debug(ident + 2);
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

    fnStmt(std::string name, std::vector<std::pair<std::string, Node::Type *>> params, Node::Type *returnType, Node::Stmt *block) : name(name), params(params), returnType(returnType), block(block) {
        kind = NodeKind::ND_FN_STMT;
    }

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "fnStmt: \n";
        Node::printIndent(ident + 1);
        std::cout << "Name: " << name << "\n";
        Node::printIndent(ident + 1);
        std::cout << "Params: \n";
        for (auto p : params) {
            Node::printIndent(ident + 2);
            std::cout << "Name: " << p.first << ", Type: ";
            p.second->debug(ident + 2);
            std::cout << "\n";
        }
        Node::printIndent(ident + 1);
        std::cout << "ReturnType: ";
        returnType->debug(ident);
        std::cout << "\n";
        Node::printIndent(ident);
        block->debug(ident + 2);
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

    void debug(int ident = 0) const override {
        Node::printIndent(ident);
        std::cout << "ReturnStmt: \n";
        Node::printIndent(ident + 1);
        std::cout << "Expr: \n";
        expr->debug(ident + 2);
    }

    ~ReturnStmt() {
        delete expr;
    }
};
