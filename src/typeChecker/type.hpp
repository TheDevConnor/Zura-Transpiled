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
extern std::string struct_name;
extern bool isType;

void handleError(int line, int pos, std::string msg, std::string note,
                  std::string typeOfError);


enum class LSPIdentifierType { // Types of identifiers that can be looked up with syntax highlighting
  Function,
  Struct,
  Enum,
  Variable,
  Type,
  Template,
  Array,
  EnumMember,
  StructMember,
  StructFunction,
  Unknown
};

struct LSPIdentifier {
  Node::Type *underlying;
  LSPIdentifierType type;
  std::string ident;
  int line;
  int pos;
};

inline std::vector<LSPIdentifier> lsp_idents = {};

struct Maps {
public:
  std::unordered_map<std::string, Node::Type *> global_symbol_table = {};
  std::unordered_map<std::string, Node::Type *> local_symbol_table = {};
  /// (fn_name, fn_return type) ->  { param name, param type }
  std::vector<std::pair<std::pair<std::string, Node::Type *>,
                        std::vector<std::pair<std::string, Node::Type *>>>>
      function_table = {};
  // store the function parameters to make sure they are not redefined
  std::vector<std::string> function_params = {};
  /// Array content table
  std::vector<Node::Type *> array_table = {};
  /// Template Table
  std::vector<std::string> template_table = {};
  std::vector<std::string> stackKeys;
  /// Struct Table
  // name -> { member name, member type }
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, Node::Type *>>>
      struct_table = {};
  /// Struct Function Table
  // struct name -> {(fn_name, fn_return type) ->  { param name, param type }}
  std::unordered_map<
      std::string,
      std::vector<std::pair<std::pair<std::string, Node::Type *>,
                            std::vector<std::pair<std::string, Node::Type *>>>>>
      struct_table_fn = {};
  /// Enum Table
  // name -> { member name, position {0} }
  std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> enum_table = {};

  enum class MathOp { Add, Subtract, Multiply, Divide, Modulo, Power };
  const std::unordered_map<std::string, MathOp> mathOps = {
      {"+", MathOp::Add},    {"-", MathOp::Subtract}, {"*", MathOp::Multiply},
      {"/", MathOp::Divide}, {"%", MathOp::Modulo},   {"^", MathOp::Power}};
  enum class BoolOp { Greater, Less, GreaterEqual, LessEqual, Equal, NotEqual };
  const std::unordered_map<std::string, BoolOp> boolOps = {
      {">", BoolOp::Greater},       {"<", BoolOp::Less},
      {">=", BoolOp::GreaterEqual}, {"<=", BoolOp::LessEqual},
      {"==", BoolOp::Equal},        {"!=", BoolOp::NotEqual}};
  enum class UnaryOP { Negate, Not, Increment, Decrement };
  const std::unordered_map<std::string, UnaryOP> unaryOps = {
      {"-", UnaryOP::Negate}, {"!", UnaryOP::Not},
      {"++", UnaryOP::Increment}, {"--", UnaryOP::Decrement}};
};

inline std::unique_ptr<Maps> map = nullptr;

std::string type_to_string(Node::Type *type);

bool isIntBasedType(Node::Type *type);

template <typename T, typename U>
static void declare(std::unordered_map<T, U> &tables, std::string name, U value,
                    int line, int pos) {
  typename std::unordered_map<T, U>::iterator res = tables.find(name);
  if (res != tables.end()) {
    std::string msg = "'" + name + "' is already defined as a '" + type_to_string(res->second) + "' in the symbol table";
    handleError(line, pos, msg, "", "Type Error");
    return;
  }
  tables[name] = value;
}

template <typename T, typename U>
static Node::Type *lookup(std::unordered_map<T, U> &tables, std::string name,
                          int line, int pos, std::string tableType) {
  typename std::unordered_map<T, U>::iterator res = (tables.find(name) != tables.end()) ? tables[name] : nullptr;
  if (res != nullptr)
    return res;

  std::string msg = "'" + name + "' is not defined in the " + tableType;
  handleError(line, pos, msg, "", tableType + " Error");
  return new SymbolType("unknown");
}

void declare_fn(Maps *maps, const std::pair<std::string, Node::Type *> &pair,
                std::vector<std::pair<std::string, Node::Type *>> paramTypes,
                int line, int pos);

void declare_struct_fn(Maps *maps, const std::pair<std::string, Node::Type *> &pair,
                       std::vector<std::pair<std::string, Node::Type *>> paramTypes,
                       int line, int pos, std::string structName);

// pair<pair<fn name, fn return type>, vector<pair<param name, param type>>>
using Fn = std::pair<std::pair<std::string, Node::Type *>,
                                       std::vector<std::pair<std::string, Node::Type *>>>;
using FnVector = std::vector<Fn>;
FnVector lookup_fn(Maps *maps, Node::Expr *callee, int line, int pos);

inline bool foundMain = false;
inline bool needsReturn = false;

inline std::shared_ptr<Node::Type> return_type = nullptr;

std::shared_ptr<SymbolType> checkReturnType(Node::Expr *expr,
                                            const std::string &defaultType);
bool checkTypeMatch(const std::shared_ptr<SymbolType> &lhs,
                    const std::shared_ptr<SymbolType> &rhs,
                    const std::string &operation, int line, int pos,
                    std::string &msg);
inline bool isLspMode = false;
void performCheck(Node::Stmt *stmt, bool isMain = true, bool isLspServer = false);
void printTables(Maps *map);

std::string determineTypeKind(Maps *map, const std::string &type);
void processStructMember(Maps *map, MemberExpr *member,
                         const std::string &name, std::string lhsType);
void processEnumMember(Maps *map, MemberExpr *member,
                        const std::string &lhsType);
void handleUnknownType(MemberExpr *member, const std::string &lhsType);

void reportOverloadedFunctionError(CallExpr *call, Node::Expr *callee);
bool validateArgumentCount(CallExpr *call, Node::Expr *callee,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams);
bool validateArgumentTypes(Maps *map, CallExpr *call, Node::Expr *callee,
                                        const std::vector<std::pair<std::string, Node::Type *>> &fnParams);

void visitArrayType(Maps *map, Node::Type *type);

void processStructFunction(Maps *map, FnStmt *fn_stmt, std::string structName);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(Maps *map, Node::Stmt *)>;
using ExprNodeHandler = std::function<void(Maps *map, Node::Expr *)>;
extern std::vector<std::pair<NodeKind, StmtNodeHandler>> stmts;
extern std::vector<std::pair<NodeKind, ExprNodeHandler>> exprs;

Node::Stmt *StmtAstLookup(Node::Stmt *node, Maps *map);
Node::Expr *ExprAstLookup(Node::Expr *node, Maps *map);

Node::Type *createDuplicate(Node::Type *type);

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
void visitMatch(Maps *map, Node::Stmt *stmt);
void visitInput(Maps *map, Node::Stmt *stmt);
void visitClose(Maps *map, Node::Stmt *stmt);

// !Expr functions
void visitTemplateCall(Maps *map, Node::Expr *expr);
void visitExpr(Maps *map, Node::Expr *expr);
void visitInt(Maps *map, Node::Expr *expr);
void visitFloat(Maps *map, Node::Expr *expr);
void visitString(Maps *map, Node::Expr *expr);
void visitChar(Maps *map, Node::Expr *expr);
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
void visitArrayAutoFill(Maps *map, Node::Expr *expr);
void visitCast(Maps *map, Node::Expr *expr);
void visitExternalCall(Maps *map, Node::Expr *expr);
void visitStructExpr(Maps *map, Node::Expr *expr);
void visitAddress(Maps *map, Node::Expr *expr);
void visitAllocMemory(Maps *map, Node::Expr *expr);
void visitFreeMemory(Maps *map, Node::Expr *expr);
void visitMemcpyMemory(Maps *map, Node::Expr *expr);
void visitSizeof(Maps *map, Node::Expr *expr);
void visitOpen(Maps *map, Node::Expr *expr);
} // namespace TypeChecker
