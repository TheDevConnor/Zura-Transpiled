#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace codegen {

using StmtHandler = std::function<void(Node::Stmt *)>;
using ExprHandler = std::function<void(Node::Expr *)>;
using TypeHandler = std::function<void(Node::Type *)>;

inline std::unordered_map<NodeKind, TypeHandler> typeHandlers;
inline std::unordered_map<NodeKind, StmtHandler> stmtHandlers;
inline std::unordered_map<NodeKind, ExprHandler> exprHandlers;


template <typename T, typename U>
T lookup(const std::unordered_map<U, T> &map, U key) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        return nullptr;
    }
    return iter->second;
}

void initMaps();

inline std::unordered_map<std::string, size_t> stackTable = {};
inline size_t stackSize;
inline std::vector<std::string> section_data = {}; 
inline std::vector<std::string> section_text = {};

void visitStmt(Node::Stmt *stmt);
void visitExpr(Node::Expr *expr);

void symbolType(Node::Type *type);
void arrayType(Node::Type *type);
void pointerType(Node::Type *type);

void program(Node::Stmt *stmt);
void constDecl(Node::Stmt *stmt);
void funcDecl(Node::Stmt *stmt);
void varDecl(Node::Stmt *stmt);
void block(Node::Stmt *stmt);
void expr(Node::Stmt *stmt);
void retrun(Node::Stmt *stmt);

void binary(Node::Expr *expr);
void unary(Node::Expr *expr);
void call(Node::Expr *expr);
void ternary(Node::Expr *expr);
void primary(Node::Expr *expr);

inline std::string output_code;
inline bool isEntryPoint = false;

void push(std::string str, bool isSectionText = false);

void gen(Node::Stmt *stmt, bool isSaved, std::string output);
} // namespace codegen
