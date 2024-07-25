#pragma once

#include "../ast/ast.hpp"
#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace TypeChecker {
inline bool foundMain = false;

/// This converts the type to a string
std::string type_to_string(Node::Type *type);
void handlerError(int line, int pos, std::string msg, std::string note,
                  std::string typeOfError);

// !Symbol Table functions
inline Node::Type *return_type = nullptr;

using NameTypePair = std::pair<std::string, Node::Type *>;
using global_symbol_table = std::vector<NameTypePair>;
using local_symbol_table = std::vector<NameTypePair>;
using function_table =
    std::unordered_map<NameTypePair, std::vector<Node::Type *>>;

template <typename T, typename U>
T table_lookup(std::unordered_map<T, U> &tables, std::string name, int line,
               int pos) {
  for (auto &pair : tables) {
    if (pair.first == name) {
      return pair.second;
    }
  }

  std::string msg = "'" + name + "' is not defined";
  handlerError(line, pos, msg, "", "Symbol Table Error");
  return new SymbolType("unknown");
}

template <typename T, typename U>
void declare(std::vector<T> &tables, std::string name, U value, int line,
             int pos) {
  for (auto &pair : tables) {
    if (pair.first == name) {
      std::string msg = "'" + name + "' is already defined";
      handlerError(line, pos, msg, "", "Symbol Table Error");
    }
  }
  tables.push_back({name, value});
}

void performCheck(Node::Stmt *stmt);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(
    global_symbol_table &gTable, local_symbol_table &lTable, Node::Stmt *)>;
using ExprNodeHandler = std::function<void(
    global_symbol_table &gTable, local_symbol_table &lTable, Node::Expr *)>;
extern std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts;
extern std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs;

Node::Stmt *StmtAstLookup(Node::Stmt *node, global_symbol_table gTable,
                          local_symbol_table lTable);
Node::Expr *ExprAstLookup(Node::Expr *node, global_symbol_table gTable,
                          local_symbol_table lTable);

// !Stmt functions
void visitStmt(global_symbol_table &gTable, local_symbol_table &lTable,
               Node::Stmt *stmt);
void visitProgram(global_symbol_table &gTable, local_symbol_table &lTable,
                  Node::Stmt *stmt);
void visitFn(global_symbol_table &gTable, local_symbol_table &lTable,
             Node::Stmt *stmt);
void visitConst(global_symbol_table &gTable, local_symbol_table &lTable,
                Node::Stmt *stmt);
void visitBlock(global_symbol_table &gTable, local_symbol_table &lTable,
                Node::Stmt *stmt);
void visitReturn(global_symbol_table &gTable, local_symbol_table &lTable,
                 Node::Stmt *stmt);

// !Expr functions
void visitExpr(global_symbol_table &gTable, local_symbol_table &lTable,
               Node::Expr *expr);
void visitNumber(global_symbol_table &gTable, local_symbol_table &lTable,
                 Node::Expr *expr);
void visitString(global_symbol_table &gTable, local_symbol_table &lTable,
                 Node::Expr *expr);
}
