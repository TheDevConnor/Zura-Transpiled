#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <string>
#include <unordered_map>

namespace codegen {
using StmtHandler = std::function<void(Node::Stmt *)>;
using ExprHandler = std::function<void(Node::Expr *)>;
using TypeHandler = std::function<void(Node::Type *)>;

inline std::unordered_map<NodeKind, TypeHandler> typeHandlers;
inline std::unordered_map<NodeKind, StmtHandler> stmtHandlers;
inline std::unordered_map<NodeKind, ExprHandler> exprHandlers;

void initMaps();

// map to ast functions
void visitStmt(Node::Stmt *stmt);
void visitExpr(Node::Expr *expr);

// Type map functions
void symbolType(Node::Type *type);
void arrayType(Node::Type *type);
void pointerType(Node::Type *type);

// Stmt map functions
void program(Node::Stmt *stmt);
void constDecl(Node::Stmt *stmt);
void funcDecl(Node::Stmt *stmt);
void varDecl(Node::Stmt *stmt);
void block(Node::Stmt *stmt);
void retrun(Node::Stmt *stmt);

// Expr map functions
void binary(Node::Expr *expr);
void unary(Node::Expr *expr);
void call(Node::Expr *expr);
void ternary(Node::Expr *expr);
void primary(Node::Expr *expr);

// Push everything to a string variable
inline std::string output_code;
void push(std::string str);

// Generate code for the given AST
void gen(Node::Stmt *stmt, bool isSaved, std::string output);
} // namespace codegen