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
std::string type_to_string(Node::Type *type);
inline Node::Type *return_type = nullptr;
void handlerError(int line, int pos, std::string msg, std::string note,
                  std::string typeOfError);

struct Maps {
  // !Symbol Table functions
  using NameTypePair = std::pair<std::string, Node::Type *>;
  using global_symbol_table = std::unordered_map<std::string, Node::Type *>;
  using local_symbol_table = std::unordered_map<std::string, Node::Type *>;
  // fn_name (fn_name, fn_return type) ->  { param name, param type }
  using function_table =
      std::vector<std::pair<NameTypePair,
                            std::vector<std::pair<std::string, Node::Type *>>>>;

  static void printTables(global_symbol_table &gTable,
                          local_symbol_table &lTable, function_table &fn_table);

  template <typename T, typename U>
  static void declare(std::unordered_map<T, U> &tables, std::string name,
                      U value, int line, int pos) {
    auto res = (tables.find(name) != tables.end()) ? tables[name] : nullptr;
    if (res != nullptr) {
      std::string msg = "'" + name + "' is already defined";
      handlerError(line, pos, msg, "", "Symbol Table Error");
      return;
    }
    tables[name] = value;
  }

  template <typename T, typename U>
  static Node::Type *lookup(std::unordered_map<T, U> &tables, std::string name,
                            int line, int pos, std::string tableType) {
    auto res = (tables.find(name) != tables.end()) ? tables[name] : nullptr;
    if (res != nullptr)
      return res;

    std::string msg = "'" + name + "' is not defined in the " + tableType;
    handlerError(line, pos, msg, "", "Symbol Table Error");
    return new SymbolType("unknown");
  }

  static void
  declare_fn(Maps::function_table &fn_table, std::string name,
             const Maps::NameTypePair &pair,
             std::vector<std::pair<std::string, Node::Type *>> paramTypes,
             int line, int pos);

  static std::pair<NameTypePair,
                   std::vector<std::pair<std::string, Node::Type *>>>
  lookup_fn(Maps::function_table &fn_table, std::string name, int line,
            int pos); 
};

void performCheck(Node::Stmt *stmt);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(
    Maps::global_symbol_table &gTable, Maps::local_symbol_table &lTable,
    Maps::function_table &fn_table, Node::Stmt *)>;
using ExprNodeHandler = std::function<void(
    Maps::global_symbol_table &gTable, Maps::local_symbol_table &lTable,
    Maps::function_table &fn_table, Node::Expr *)>;
extern std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts;
extern std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs;

Node::Stmt *StmtAstLookup(Node::Stmt *node, Maps::global_symbol_table gTable,
                          Maps::local_symbol_table lTable,
                          Maps::function_table fn_table);
Node::Expr *ExprAstLookup(Node::Expr *node, Maps::global_symbol_table gTable,
                          Maps::local_symbol_table lTable,
                          Maps::function_table fn_table);

// !Stmt functions
void visitStmt(Maps::global_symbol_table &gTable,
               Maps::local_symbol_table &lTable, Maps::function_table &fn_table,
               Node::Stmt *stmt);
void visitProgram(Maps::global_symbol_table &gTable,
                  Maps::local_symbol_table &lTable,
                  Maps::function_table &fn_table, Node::Stmt *stmt);
void visitFn(Maps::global_symbol_table &gTable,
             Maps::local_symbol_table &lTable, Maps::function_table &fn_table,
             Node::Stmt *stmt);
void visitConst(Maps::global_symbol_table &gTable,
                Maps::local_symbol_table &lTable,
                Maps::function_table &fn_table, Node::Stmt *stmt);
void visitBlock(Maps::global_symbol_table &gTable,
                Maps::local_symbol_table &lTable,
                Maps::function_table &fn_table, Node::Stmt *stmt);
void visitVar(Maps::global_symbol_table &gTable,
              Maps::local_symbol_table &lTable, Maps::function_table &fn_table,
              Node::Stmt *stmt);
void visitReturn(Maps::global_symbol_table &gTable,
                 Maps::local_symbol_table &lTable,
                 Maps::function_table &fn_table, Node::Stmt *stmt);

// !Expr functions
void visitExpr(Maps::global_symbol_table &gTable,
               Maps::local_symbol_table &lTable, Maps::function_table &fn_table,
               Node::Expr *expr);
void visitNumber(Maps::global_symbol_table &gTable,
                 Maps::local_symbol_table &lTable,
                 Maps::function_table &fn_table, Node::Expr *expr);
void visitString(Maps::global_symbol_table &gTable,
                 Maps::local_symbol_table &lTable,
                 Maps::function_table &fn_table, Node::Expr *expr);
void visitIdent(Maps::global_symbol_table &gTable,
                Maps::local_symbol_table &lTable,
                Maps::function_table &fn_table, Node::Expr *expr);
void visitBinary(Maps::global_symbol_table &gTable,
                 Maps::local_symbol_table &lTable,
                 Maps::function_table &fn_table, Node::Expr *expr);
void visitCall(Maps::global_symbol_table &gTable,
               Maps::local_symbol_table &lTable, Maps::function_table &fn_table,
               Node::Expr *expr);
} // namespace TypeChecker
