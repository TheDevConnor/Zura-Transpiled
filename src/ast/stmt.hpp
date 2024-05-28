#pragma once

#include <vector>

#include "ast.hpp"

class ProgramStmt : public Node::Stmt {
    std::vector<Node::Stmt *> stmt;

    ProgramStmt(std::vector<Node::Stmt *> stmt) : stmt(stmt) {
        kind = NodeKind::ND_PROGRAM;
    }
};

class ExprStmt : public Node::Stmt {
    Node::Expr *expr;

    ExprStmt(Node::Expr *expr) : expr(expr) {
        kind = NodeKind::ND_EXPR_STMT;
    }
};