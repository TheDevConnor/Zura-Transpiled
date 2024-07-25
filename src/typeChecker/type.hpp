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
inline bool foundMain = false;

// !Symbol Table functions
inline Node::Type *return_type = nullptr;
using symbol_table = std::unordered_map<std::string, Node::Type *>;
void declare(symbol_table &table, std::string name, Node::Type *type, int line,
             int pos);
Node::Type *table_lookup(symbol_table &table, std::string name, int line,
                         int pos);

using callables_table =
    std::unordered_map<std::string,
                       std::vector<std::pair<std::string, Node::Type *>>>;
void declare(callables_table &table, std::string name,
             std::vector<std::pair<std::string, Node::Type *>> params);
std::vector<std::pair<std::string, Node::Type *>>
table_lookup(callables_table &table, std::string name, int line, int pos);

void check(Node::Stmt *stmt);

/// This converts the type to a string
std::string type_to_string(Node::Type *type);
void handlerError(int line, int pos, std::string msg, std::string note);

// !Ast Visitor functions
void visitStmt(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitExpr(callables_table &ctable, symbol_table &table, Node::Expr *expr);

void visitProgram(callables_table &ctable, symbol_table &table,
                  Node::Stmt *stmt);
void visitReturn(callables_table &ctable, symbol_table &table,
                 Node::Stmt *stmt);
void visitConst(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitBlock(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitVar(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitFn(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);
void visitImport(callables_table &ctable, symbol_table &table, Node::Stmt *stmt);

void visitNumber(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitString(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitIdent(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitCall(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitBinary(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitUnary(callables_table &ctable, symbol_table &table, Node::Expr *expr);
void visitGrouping(callables_table &ctable, symbol_table &table, Node::Expr *expr);

// !TypeChecker functions
using StmtNodeHandler =
    std::function<void(callables_table &ctables, symbol_table &, Node::Stmt *)>;
using ExprNodeHandler =
    std::function<void(callables_table &ctables, symbol_table &, Node::Expr *)>;
extern std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts;
extern std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs;

void lookup(callables_table &ctables, symbol_table &table, Node::Stmt *stmt);
void lookup(callables_table &ctables, symbol_table &table, Node::Expr *expr);
} // namespace TypeChecker
