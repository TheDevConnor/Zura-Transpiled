#pragma once

#include "../ast/ast.hpp"
#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace TypeChecker {
// !Symbol Table functions
inline Node::Type *return_type = nullptr;
using symbol_table = std::unordered_map<std::string, Node::Type *>;
void declare(symbol_table &table, std::string name, Node::Type *type);
Node::Type *table_lookup(symbol_table &table, std::string name);

using callables_table = std::unordered_map<std::string, std::vector<std::pair<std::string, Node::Type *>>>;
void declare(callables_table &table, std::string name, std::vector<std::pair<std::string, Node::Type *>> params);
std::vector<std::pair<std::string, Node::Type *>> table_lookup(callables_table &table, std::string name);

void check(Node::Stmt *stmt);

/// This converts the type to a string
std::string type_to_string(Node::Type *type);

// !Ast Visitor functions
void visitStmt(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitExpr(callables_table &ctable, symbol_table &table, Node::Expr *expr);

void visitProgram(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitReturn(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitConst(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitBlock(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitVar(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitFn(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(callables_table &ctables, symbol_table &, Node::Stmt *)>;
using ExprNodeHandler = std::function<void(callables_table &ctables, symbol_table &, Node::Expr *)>;
static std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts = {
    {NodeKind::ND_PROGRAM, visitProgram},
    {NodeKind::ND_FN_STMT, visitFn},
    {NodeKind::ND_CONST_STMT, visitConst},
    {NodeKind::ND_BLOCK_STMT, visitBlock},
    {NodeKind::ND_VAR_STMT, visitVar},
    {NodeKind::ND_RETURN_STMT, visitReturn},
    {NodeKind::ND_EXPR_STMT,
     [](callables_table &ctables, symbol_table &table, Node::Stmt *stmt) {
       auto expr_stmt = static_cast<ExprStmt *>(stmt);
       visitExpr(ctables, table, expr_stmt->expr);
     }},
};


static std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs = {
  {NodeKind::ND_NUMBER,
    [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
        auto number = static_cast<NumberExpr *>(expr);
        return_type = new SymbolType("int");
    }},
  {NodeKind::ND_STRING,
    [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
        auto string = static_cast<StringExpr *>(expr);
        return_type = new SymbolType("string");
    }},
  {NodeKind::ND_IDENT,
    [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
        auto identifier = static_cast<IdentExpr *>(expr);
        return_type = table_lookup(table, identifier->name);
    }},
  {NodeKind::ND_BINARY,
    [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
        auto binary = static_cast<BinaryExpr *>(expr);
        visitExpr(ctables, table, binary->lhs);
        visitExpr(ctables, table, binary->rhs);
    }},
  {NodeKind::ND_CALL,
    [](callables_table &ctables, symbol_table &table, Node::Expr *expr) {
        auto call = static_cast<CallExpr *>(expr);
        
        visitExpr(ctables, table, call->callee);

        auto params = table_lookup(ctables, static_cast<IdentExpr *>(call->callee)->name);

        if (params.size() != call->args.size()) {
            std::string msg = "Function '" + static_cast<IdentExpr *>(call->callee)->name + "' expects " + std::to_string(params.size()) + " arguments but got " + std::to_string(call->args.size());
            std::cout << msg << std::endl;
        }

        for (int i = 0; i < params.size(); i++) {
            visitExpr(ctables, table, call->args[i]);
            if (type_to_string(params[i].second) != type_to_string(return_type)) {
                std::string msg = "Function '" + static_cast<IdentExpr *>(call->callee)->name + "' expects argument " + std::to_string(i) + " to be of type " + type_to_string(params[i].second) + " but got " + type_to_string(return_type);
                std::cout << msg << std::endl;
            }
        }

        return_type = table_lookup(table, static_cast<IdentExpr *>(call->callee)->name);
    }},
};

void lookup(callables_table &ctables,symbol_table &table, Node::Stmt *stmt);
void lookup(callables_table &ctables,symbol_table &table, Node::Expr *expr);
} 
