#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "../ast/ast.hpp"
#include "../helper/error/error.hpp"
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
} // namespace Parser

struct Parser::PStruct {
  std::vector<Lexer::Token> tks;
  std::string current_file;
  int pos = 0;

  Lexer::Token current(PStruct *psr) { return psr->tks[psr->pos]; }

  Lexer::Token advance(PStruct *psr) {
    auto tk = current(psr);
    psr->pos++;
    return tk;
  }

  Lexer::Token peek(PStruct *psr, int offset = 0) {
    return psr->tks[psr->pos + offset];
  }

  bool hadTokens(PStruct *psr) { return psr->pos < psr->tks.size(); }

  Lexer::Token expect(PStruct *psr, TokenKind tk, std::string msg) {
    Lexer lexer;
    if (peek(psr).kind == TokenKind::END_OF_FILE) {
      ErrorClass::error(current(psr).line, current(psr).column, msg, "",
                        "Parser Error", psr->current_file, lexer, psr->tks, true, false,
                        false, false, false, false);
      ErrorClass::printError();
      exit(1);
    }
    if (current(psr).kind != tk) {
      ErrorClass::error(current(psr).line, current(psr).column, msg, "",
                        "Parser Error", psr->current_file, lexer, psr->tks, true, false,
                        false, false, false, false);
      return current(psr);
    }

    return advance(psr);
  }
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
static std::vector<TokenKind> ignore_tokens;
void createMaps();

Node::Expr *led(PStruct *psr, Node::Expr *left, BindingPower bp);
BindingPower getBP(TokenKind tk);
Node::Stmt *stmt(PStruct *psr, std::string name);
Node::Expr *nud(PStruct *psr);
bool isIgnoreToken(TokenKind tk);

// Maps for the Pratt Parser for types.
using TypeNudHandler = std::function<Node::Type *(PStruct *)>;
using TypeLedHandler =
    std::function<Node::Type *(PStruct *, Node::Type *, BindingPower)>;

static std::vector<std::pair<TokenKind, TypeNudHandler>> type_nud_lu;
static std::vector<std::pair<TokenKind, TypeLedHandler>> type_led_lu;
static std::vector<std::pair<TokenKind, BindingPower>> type_bp_lu;
void createTypeMaps();

Node::Type *type_led(PStruct *psr, Node::Type *left, BindingPower bp);
BindingPower type_getBP(PStruct *psr, TokenKind tk);
Node::Type *type_nud(PStruct *psr);

Node::Type *symbol_table(PStruct *psr);
Node::Type *array_type(PStruct *psr);
Node::Type *parseType(PStruct *psr, BindingPower bp);
Node::Type *pointer_type(PStruct *psr);

// Pratt parser functions.
PStruct *setupParser(PStruct *psr, Lexer *lex, Lexer::Token tk, std::string current_file);
Node::Expr *parseExpr(PStruct *psr, BindingPower bp);

// Expr Functions
Node::Expr *primary(PStruct *psr);
Node::Expr *unary(PStruct *psr);
Node::Expr *_prefix(PStruct *psr);
Node::Expr *group(PStruct *psr);
Node::Expr *array(PStruct *psr);
Node::Expr *bool_expr(PStruct *psr);
Node::Expr *cast_expr(PStruct *psr);
Node::Expr *_postfix(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *binary(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *assign(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *parse_call(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_ternary(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *_member(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *index(PStruct *psr, Node::Expr *left, BindingPower bp);
Node::Expr *resolution(PStruct *psr, Node::Expr *left, BindingPower bp);

// Stmt Functions
Node::Stmt *returnStmt(PStruct *psr, std::string name);
Node::Stmt *structStmt(PStruct *psr, std::string name);
Node::Stmt *importStmt(PStruct *psr, std::string name);
Node::Stmt *parseStmt(PStruct *psr, std::string name);
Node::Stmt *blockStmt(PStruct *psr, std::string name);
Node::Stmt *constStmt(PStruct *psr, std::string name);
Node::Stmt *enumStmt(PStruct *psr, std::string name);
Node::Stmt *loopStmt(PStruct *psr, std::string name);
Node::Stmt *printStmt(PStruct *psr, std::string name);
Node::Stmt *varStmt(PStruct *psr, std::string name);
Node::Stmt *funStmt(PStruct *psr, std::string name);
Node::Stmt *ifStmt(PStruct *psr, std::string name);
Node::Stmt *templateStmt(PStruct *psr, std::string name);
Node::Stmt *breakStmt(PStruct *psr, std::string name);
Node::Stmt *continueStmt(PStruct *psr, std::string name);
Node::Stmt *exprStmt(PStruct *psr);
} // namespace Parser
