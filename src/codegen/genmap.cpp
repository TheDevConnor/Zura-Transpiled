#include "gen.hpp"

void codegen::initMaps() {
  typeHandlers = {
      {ND_SYMBOL_TYPE, symbolType},
      {ND_ARRAY_TYPE, arrayType},
      {ND_POINTER_TYPE, pointerType},
  };
  stmtHandlers = {
      {ND_PROGRAM, program},         {ND_CONST_STMT, constDecl},
      {ND_VAR_STMT, varDecl},        {ND_FN_STMT, funcDecl},
      {ND_BLOCK_STMT, block},        {ND_RETURN_STMT, _return},
      {ND_IF_STMT, ifStmt},          {ND_EXPR_STMT, expr},
      {ND_PRINT_STMT, print},        {ND_WHILE_STMT, whileLoop},
      {ND_FOR_STMT, forLoop},        {ND_BREAK_STMT, _break},
      {ND_CONTINUE_STMT, _continue},
  };
  exprHandlers = {
      {ND_BINARY, binary},   {ND_UNARY, unary},      {ND_CALL, call},
      {ND_TERNARY, ternary}, {ND_INT, primary},      {ND_FLOAT, primary},
      {ND_IDENT, primary},   {ND_STRING, primary},   {ND_BOOL, primary},
      {ND_GROUP, grouping},  {ND_ASSIGN, assign},    {ND_PREFIX, unary},
      {ND_POSTFIX, unary},   {ND_ARRAY, _arrayExpr}, {ND_INDEX, arrayElem},
  };
  opMap = {
      {"+", "add"},  {"-", "sub"},    {"*", "imul"},   {"/", "idiv"},
      {"%", "mod"}, {"==", "sete"},  {"!=", "setne"}, {"<", "setl"},
      {">", "setg"}, {"<=", "setle"}, {">=", "setge"}, {"&&", "and"},
      {"||", "or"}, {"^", "power"},
  };
}