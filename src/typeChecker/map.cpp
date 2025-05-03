#include <functional>
#include <unordered_map>

#include "../ast/ast.hpp"
#include "type.hpp"

using namespace TypeChecker;

void TypeChecker::initMaps() {
  stmts = {
      {NodeKind::ND_PROGRAM, visitProgram},
      {NodeKind::ND_CONST_STMT, visitConst},
      {NodeKind::ND_FN_STMT, visitFn},
      {NodeKind::ND_BLOCK_STMT, visitBlock},
      {NodeKind::ND_STRUCT_STMT, visitStruct},
      {NodeKind::ND_ENUM_STMT, visitEnum},
      {NodeKind::ND_RETURN_STMT, visitReturn},
      {NodeKind::ND_VAR_STMT, visitVar},
      {NodeKind::ND_IF_STMT, visitIf},
      {NodeKind::ND_EXPR_STMT, visitExprStmt},
      {NodeKind::ND_PRINT_STMT, visitPrint},
      {NodeKind::ND_WHILE_STMT, visitWhile},
      {NodeKind::ND_FOR_STMT, visitFor},
      {NodeKind::ND_BREAK_STMT, visitBreak},
      {NodeKind::ND_CONTINUE_STMT, visitContinue},
      {NodeKind::ND_IMPORT_STMT, visitImport},
      {NodeKind::ND_LINK_STMT, visitLink},
      {NodeKind::ND_EXTERN_STMT, visitExtern},
      {NodeKind::ND_MATCH_STMT, visitMatch},
      {NodeKind::ND_INPUT_STMT, visitInput},
      {NodeKind::ND_CLOSE, visitClose},
  };

  exprs = {
      {NodeKind::ND_INT, visitInt},
      {NodeKind::ND_FLOAT, visitFloat},
      {NodeKind::ND_IDENT, visitIdent},
      {NodeKind::ND_STRING, visitString},
      {NodeKind::ND_BOOL, visitBool},
      {NodeKind::ND_CHAR, visitChar},
      {NodeKind::ND_BINARY, visitBinary},
      {NodeKind::ND_CALL, visitCall},
      {NodeKind::ND_TERNARY, visitTernary},
      {NodeKind::ND_GROUP, visitGrouping},
      {NodeKind::ND_UNARY, visitUnary},
      {NodeKind::ND_MEMBER, visitMember},
      {NodeKind::ND_ASSIGN, visitAssign},
      {NodeKind::ND_ARRAY, visitArray},
      {NodeKind::ND_INDEX, visitIndex},
      {NodeKind::ND_ARRAY_AUTO_FILL, visitArrayAutoFill},
      {NodeKind::ND_PREFIX, visitUnary},
      {NodeKind::ND_POSTFIX, visitUnary},
      {NodeKind::ND_CAST, visitCast},
      {NodeKind::ND_TEMPLATE_CALL, visitTemplateCall},
      {NodeKind::ND_EXTERNAL_CALL, visitExternalCall},
      {NodeKind::ND_STRUCT, visitStructExpr},
      {NodeKind::ND_ADDRESS, visitAddress},
      {NodeKind::ND_DEREFERENCE, visitDereference},
      {NodeKind::ND_FREE_MEMORY, visitFreeMemory},
      {NodeKind::ND_ALLOC_MEMORY, visitAllocMemory},
      {NodeKind::ND_SIZEOF, visitSizeof},
      {NodeKind::ND_MEMCPY_MEMORY, visitMemcpyMemory},
      {NodeKind::ND_OPEN, visitOpen},
      {NodeKind::ND_GETARGC, visitArgc},
      {NodeKind::ND_GETARGV, visitArgv},
      {NodeKind::ND_STRCMP, visitStrcmp},
  };
}

Node::Stmt *TypeChecker::StmtAstLookup(Node::Stmt *node) {
  auto res = stmts.find(node->kind);
  if (res != stmts.end())
    res->second(node);
  return node;
}

Node::Expr *TypeChecker::ExprAstLookup(Node::Expr *node) {
  auto res = exprs.find(node->kind);
  if (res != exprs.end())
    res->second(node);
  return node;
}

void TypeChecker::printTables() {}
