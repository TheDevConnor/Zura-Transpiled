#include "gen.hpp"

void codegen::initMaps() {
  typeHandlers = {
      {ND_SYMBOL_TYPE, symbolType},
      {ND_ARRAY_TYPE, arrayType},
      {ND_POINTER_TYPE, pointerType},
  };
  stmtHandlers = {
      {ND_PROGRAM, program},  {ND_CONST_STMT, constDecl},
      {ND_VAR_STMT, varDecl}, {ND_FN_STMT, funcDecl},
      {ND_BLOCK_STMT, block}, {ND_RETURN_STMT, retrun},
      {ND_IF_STMT, ifStmt},   {ND_EXPR_STMT, expr},
  };
  exprHandlers = {
      {ND_BINARY, binary},   {ND_UNARY, unary},    {ND_CALL, call},
      {ND_TERNARY, ternary}, {ND_NUMBER, primary}, {ND_IDENT, primary},
      {ND_STRING, primary},  {ND_BOOL, primary},   {ND_GROUP, grouping},
      {ND_ASSIGN, assign}
  };
}