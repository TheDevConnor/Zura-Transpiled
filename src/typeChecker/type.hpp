#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

#include "../ast/ast.hpp"
#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"

inline size_t struct_size;

namespace TypeChecker {
extern std::string struct_name;
extern bool isType;
inline std::set<std::string> importedFiles;

void handleError(int line, int pos, std::string msg, std::string note,
                 std::string typeOfError, int endPos = 0);

enum class LSPIdentifierType { // Types of identifiers that can be looked up
                               // with syntax highlighting
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
  size_t line;
  size_t pos;
  size_t fileID;
};

inline std::vector<LSPIdentifier> lsp_idents = {};
enum class MathOp { Add, Subtract, Multiply, Divide, Modulo, Power};
const std::unordered_map<std::string, MathOp> mathOps = {
    {"+", MathOp::Add},    {"-", MathOp::Subtract}, {"*", MathOp::Multiply},
    {"/", MathOp::Divide}, {"%", MathOp::Modulo},   {"^", MathOp::Power}};
enum class BoolOp { Greater, Less, GreaterEqual, LessEqual, Equal, NotEqual };
const std::unordered_map<std::string, BoolOp> boolOps = {
    {">", BoolOp::Greater},       {"<", BoolOp::Less},
    {">=", BoolOp::GreaterEqual}, {"<=", BoolOp::LessEqual},
    {"==", BoolOp::Equal},        {"!=", BoolOp::NotEqual}};
enum class LogicOp { And, Or };
const std::unordered_map<std::string, LogicOp> logicOps = {
    {"&&", LogicOp::And}, {"||", LogicOp::Or}};
enum class UnaryOP { Negate, Not, Increment, Decrement };
const std::unordered_map<std::string, UnaryOP> unaryOps = {
    {"-", UnaryOP::Negate},
    {"!", UnaryOP::Not},
    {"++", UnaryOP::Increment},
    {"--", UnaryOP::Decrement}};

inline bool foundMain = false;
inline bool needsReturn = false;
inline bool isLspMode = false;

inline std::shared_ptr<Node::Type> return_type = nullptr;

std::string type_to_string(Node::Type *type);

bool isIntBasedType(Node::Type *type);

bool checkTypeMatch(Node::Type *lhs, Node::Type *rhs);
void performCheck(Node::Stmt *stmt, bool isMain = true,
                  bool isLspServer = false);
void printTables(void);

std::string determineTypeKind(const std::string &type);

std::shared_ptr<Node::Type> share(Node::Type *type);
void processStructMember(MemberExpr *member, const std::string &name,
                         std::string lhsType);
void processEnumMember(MemberExpr *member, const std::string &lhsType);
void handleUnknownType(MemberExpr *member, const std::string &lhsType);

void reportOverloadedFunctionError(CallExpr *call, Node::Expr *callee);
bool validateArgumentCount(
    CallExpr *call, Node::Expr *callee,
    const std::unordered_map<std::string, Node::Type *> &fnParams);
bool validateArgumentTypes(
    CallExpr *call, Node::Expr *callee,
    const std::unordered_map<std::string, Node::Type *> &fnParams);

void visitArrayType(Node::Type *type);

void processStructFunction(FnStmt *fn_stmt, std::string structName);

// !TypeChecker functions
using StmtNodeHandler = std::function<void(Node::Stmt *)>;
using ExprNodeHandler = std::function<void(Node::Expr *)>;
inline std::unordered_map<NodeKind, StmtNodeHandler> stmts;
inline std::unordered_map<NodeKind, ExprNodeHandler> exprs;
void initMaps(void);

Node::Stmt *StmtAstLookup(Node::Stmt *node);
Node::Expr *ExprAstLookup(Node::Expr *node);

Node::Type *createDuplicate(Node::Type *type);

// !Stmt functions
void visitStmt(Node::Stmt *stmt);
void visitExprStmt(Node::Stmt *stmt);
void visitProgram(Node::Stmt *stmt);
void visitFn(Node::Stmt *stmt);
void visitConst(Node::Stmt *stmt);
void visitStruct(Node::Stmt *stmt);
void visitEnum(Node::Stmt *stmt);
void visitBlock(Node::Stmt *stmt);
void visitVar(Node::Stmt *stmt);
void visitPrint(Node::Stmt *stmt);
void visitIf(Node::Stmt *stmt);
void visitReturn(Node::Stmt *stmt);
void visitWhile(Node::Stmt *stmt);
void visitFor(Node::Stmt *stmt);
void visitBreak(Node::Stmt *stmt);
void visitContinue(Node::Stmt *stmt);
void visitImport(Node::Stmt *stmt);
void visitLink(Node::Stmt *stmt);
void visitExtern(Node::Stmt *stmt);
void visitMatch(Node::Stmt *stmt);
void visitInput(Node::Stmt *stmt);
void visitClose(Node::Stmt *stmt);

// !Expr functions
void visitTemplateCall(Node::Expr *expr);
void visitExpr(Node::Expr *expr);
void visitInt(Node::Expr *expr);
void visitFloat(Node::Expr *expr);
void visitString(Node::Expr *expr);
void visitChar(Node::Expr *expr);
void visitIdent(Node::Expr *expr);
void visitBinary(Node::Expr *expr);
void visitUnary(Node::Expr *expr);
void visitBool(Node::Expr *expr);
void visitGrouping(Node::Expr *expr);
void visitCall(Node::Expr *expr);
void visitTernary(Node::Expr *expr);
void visitMember(Node::Expr *expr);
void visitAssign(Node::Expr *expr);
void visitArray(Node::Expr *expr);
void visitIndex(Node::Expr *expr);
void visitArrayAutoFill(Node::Expr *expr);
void visitCast(Node::Expr *expr);
void visitExternalCall(Node::Expr *expr);
void visitStructExpr(Node::Expr *expr);
void visitAddress(Node::Expr *expr);
void visitDereference(Node::Expr *expr);
void visitAllocMemory(Node::Expr *expr);
void visitFreeMemory(Node::Expr *expr);
void visitMemcpyMemory(Node::Expr *expr);
void visitSizeof(Node::Expr *expr);
void visitOpen(Node::Expr *expr);
void visitArgc(Node::Expr *expr);
void visitArgv(Node::Expr *expr);
void visitStrcmp(Node::Expr *expr);
} // namespace TypeChecker
