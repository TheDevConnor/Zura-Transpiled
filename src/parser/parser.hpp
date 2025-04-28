#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"

namespace Parser {
enum BindingPower {
  defaultValue = 0,
  comma = 1,
  assignment = 2,
  ternary = 3,
  logicalOr = 4,
  logicalAnd = 5,
  relational = 6,
  comparison = 7,
  additive = 8,
  multiplicative = 9,
  range = 9,
  power = 10,
  prefix = 11,
  postfix = 12,
  call = 13,
  member = 13,
  _primary = 14,
  err = 15
};
struct PStruct;
inline Lexer lexer;
}  // namespace Parser

struct Parser::PStruct {
  std::vector<Lexer::Token> tks;
  std::string current_file;
  size_t pos = 0;

  Lexer::Token current();
  Lexer::Token advance();
  Lexer::Token peek(int offset = 0);
  Lexer::Token expect(TokenKind tk, std::string msg);

  bool hadTokens();
};

namespace Parser {
template <typename T, typename U>
T lookup(PStruct *psr, const std::vector<std::pair<U, T>> &lu, U key);

Node::Stmt *parse(const char *source, std::string file);

// Maps for the Pratt Parser
using StmtHandler = std::function<Node::Stmt *(PStruct *, std::string)>;
using NudHandler = std::function<Node::Expr *(PStruct *)>;
using LedHandler =
    std::function<Node::Expr *(PStruct *, Node::Expr *, BindingPower)>;

// Maps for the Pratt Parser for statements and expressions.
static std::vector<std::pair<TokenKind, StmtHandler>> stmt_lu;
static std::vector<std::pair<TokenKind, NudHandler>> nud_lu;
static std::vector<std::pair<TokenKind, LedHandler>> led_lu;
static std::vector<std::pair<TokenKind, BindingPower>> bp_lu;
void createMaps(void);

Node::Expr *led(PStruct *psr, Node::Expr *left, BindingPower bp);
BindingPower getBP(TokenKind tk);
Node::Stmt *stmt(PStruct *psr, std::string name);
Node::Expr *nud(PStruct *psr);
;

// Maps for the Pratt Parser for types.
using TypeNudHandler = std::function<Node::Type *(PStruct *)>;
using TypeLedHandler =
    std::function<Node::Type *(PStruct *, Node::Type *, BindingPower)>;

static std::vector<std::pair<TokenKind, TypeNudHandler>> type_nud_lu;
static std::vector<std::pair<TokenKind, TypeLedHandler>> type_led_lu;
static std::vector<std::pair<TokenKind, BindingPower>> type_bp_lu;
void createTypeMaps(void);

Node::Type *type_led(PStruct *psr, Node::Type *left, BindingPower bp);
BindingPower type_getBP(PStruct *psr, TokenKind tk);
Node::Type *type_nud(PStruct *psr);

Node::Type *symbol_table(PStruct *psr);
Node::Type *array_type(PStruct *psr);
Node::Type *parseType(PStruct *psr);
Node::Type *pointer_type(PStruct *psr);
Node::Expr *nullType(PStruct *psr);
Node::Type *type_application(PStruct *psr);
Node::Type *function_type(PStruct *psr);

// Pratt parser functions.
Node::Expr *parseExpr(PStruct *psr, BindingPower bp);

// Expr Functions
Node::Expr *primary(PStruct *psr);
Node::Expr *unary(PStruct *psr);
Node::Expr *_prefix(PStruct *psr);
Node::Expr *group(PStruct *psr);
Node::Expr *array(PStruct *psr);
Node::Expr *boolExpr(PStruct *psr);
Node::Expr *castExpr(PStruct *psr);
Node::Expr *externalCall(PStruct *psr);
Node::Expr *structExpr(PStruct *psr);
Node::Expr *address(PStruct *psr);
Node::Expr *allocExpr(PStruct *psr);
Node::Expr *freeExpr(PStruct *psr);
Node::Expr *sizeofExpr(PStruct *psr);
Node::Expr *memcpyExpr(PStruct *psr);
Node::Expr *openExpr(PStruct *psr);
// Binary Functions
Node::Expr *_postfix(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *binary(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *assign(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *parse_call(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_ternary(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_member(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *index(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *resolution(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *dereference(PStruct *psr, Node::Expr *left, BindingPower bp);
// Stmt Functions
Node::Stmt *matchStmt(PStruct *psr, std::string name);
Node::Stmt *returnStmt(PStruct *psr, std::string name);
Node::Stmt *structStmt(PStruct *psr, std::string name);
Node::Stmt *importStmt(PStruct *psr, std::string name);
Node::Stmt *parseStmt(PStruct *psr, std::string name);
Node::Stmt *blockStmt(PStruct *psr, std::string name);
Node::Stmt *constStmt(PStruct *psr, std::string name);
Node::Stmt *enumStmt(PStruct *psr, std::string name);
Node::Stmt *loopStmt(PStruct *psr, std::string name);
Node::Stmt *printStmt(PStruct *psr, std::string name);
Node::Stmt *printlnStmt(PStruct *psr, std::string name);
Node::Stmt *varStmt(PStruct *psr, std::string name);
Node::Stmt *funStmt(PStruct *psr, std::string name);
Node::Stmt *ifStmt(PStruct *psr, std::string name);
Node::Stmt *templateStmt(PStruct *psr, std::string name);
Node::Stmt *breakStmt(PStruct *psr, std::string name);
Node::Stmt *continueStmt(PStruct *psr, std::string name);
Node::Stmt *linkStmt(PStruct *psr, std::string name);
Node::Stmt *externStmt(PStruct *psr, std::string name);
Node::Stmt *inputStmt(PStruct *psr, std::string name);
Node::Stmt *closeStmt(PStruct *psr, std::string name);
Node::Stmt *getArgc(PStruct *psr, std::string name);
Node::Stmt *getArgv(PStruct *psr, std::string name);

Node::Stmt *exprStmt(PStruct *psr);
}  // namespace Parser
