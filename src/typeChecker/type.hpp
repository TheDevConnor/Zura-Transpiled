#pragma once

#include "../ast/ast.hpp"
#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace TypeChecker {
inline bool foundMain = false;
std::string type_to_string(Node::Type *type);
inline std::shared_ptr<Node::Type> return_type = nullptr;
void handlerError(int line, int pos, std::string msg, std::string note,
                  std::string typeOfError);

struct Maps {
public:
  // !Symbol Table functions
  using NameTypePair = std::pair<std::string, Node::Type *>;

  std::unordered_map<std::string, Node::Type *> global_symbol_table;
  std::unordered_map<std::string, Node::Type *> local_symbol_table;
  /// fn_name (fn_name, fn_return type) ->  { param name, param type }
  std::vector<std::pair<NameTypePair,
                        std::vector<std::pair<std::string, Node::Type *>>>>
      function_table;

  // Template Table
  std::unordered_map<std::string, Node::Type *> template_table;

  // Array content table
  std::vector<Node::Type *> array_table;

  template <typename T, typename U>
  static void declare(std::unordered_map<T, U> &tables, std::string name,
                      U value, int line, int pos) {
    auto res = (tables.find(name) != tables.end()) ? tables[name] : nullptr;
    if (res != nullptr) {
      std::string msg = "'" + name + "' is already defined as an '" +
                        type_to_string(res) + "' in the symbol table";
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
  declare_fn(Maps *maps, std::string name, const Maps::NameTypePair &pair,
             std::vector<std::pair<std::string, Node::Type *>> paramTypes,
             int line, int pos);

  static std::pair<NameTypePair,
                   std::vector<std::pair<std::string, Node::Type *>>>
  lookup_fn(Maps *maps, std::string name, int line, int pos);
};

inline bool needsReturn = false;

void performCheck(Node::Stmt *stmt, bool isMain = true);
void printTables(Maps *map);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(Maps *map, Node::Stmt *)>;
using ExprNodeHandler = std::function<void(Maps *map, Node::Expr *)>;
extern std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts;
extern std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs;

Node::Stmt *StmtAstLookup(Node::Stmt *node, Maps *map);
Node::Expr *ExprAstLookup(Node::Expr *node, Maps *map);

// !Stmt functions
void visitStmt(Maps *map, Node::Stmt *stmt);
void visitExprStmt(Maps *map, Node::Stmt *stmt);
void visitProgram(Maps *map, Node::Stmt *stmt);
void visitFn(Maps *map, Node::Stmt *stmt);
void visitConst(Maps *map, Node::Stmt *stmt);
void visitStruct(Maps *map, Node::Stmt *stmt);
void visitEnum(Maps *map, Node::Stmt *stmt);
void visitBlock(Maps *map, Node::Stmt *stmt);
void visitVar(Maps *map, Node::Stmt *stmt);
void visitPrint(Maps *map, Node::Stmt *stmt);
void visitIf(Maps *map, Node::Stmt *stmt);
void visitTemplateStmt(Maps *map, Node::Stmt *stmt);
void visitReturn(Maps *map, Node::Stmt *stmt);
void visitWhile(Maps *map, Node::Stmt *stmt);
void visitFor(Maps *map, Node::Stmt *stmt);
void visitBreak(Maps *map, Node::Stmt *stmt);
void visitContinue(Maps *map, Node::Stmt *stmt);
void visitImport(Maps *map, Node::Stmt *stmt);
void visitLink(Maps *map, Node::Stmt *stmt);
void visitExtern(Maps *map, Node::Stmt *stmt);

// !Expr functions
void visitTemplateCall(Maps *map, Node::Expr *expr);  
void visitExpr(Maps *map, Node::Expr *expr);
void visitInt(Maps *map, Node::Expr *expr);
void visitFloat(Maps *map, Node::Expr *expr);
void visitString(Maps *map, Node::Expr *expr);
void visitIdent(Maps *map, Node::Expr *expr);
void visitBinary(Maps *map, Node::Expr *expr);
void visitUnary(Maps *map, Node::Expr *expr);
void visitBool(Maps *map, Node::Expr *expr);
void visitGrouping(Maps *map, Node::Expr *expr);
void visitCall(Maps *map, Node::Expr *expr);
void visitTernary(Maps *map, Node::Expr *expr);
void visitMember(Maps *map, Node::Expr *expr);
void visitAssign(Maps *map, Node::Expr *expr);
void visitArray(Maps *map, Node::Expr *expr);
void visitIndex(Maps *map, Node::Expr *expr);
void visitCast(Maps *map, Node::Expr *expr);
void visitExternalCall(Maps *map, Node::Expr *expr);
} // namespace TypeChecker
