#include "gen.hpp"

void codegen::initMaps() {
  stmtHandlers = {
      {ND_PROGRAM, program},         {ND_CONST_STMT, constDecl},
      {ND_VAR_STMT, varDecl},        {ND_FN_STMT, funcDecl},
      {ND_BLOCK_STMT, block},        {ND_RETURN_STMT, _return},
      {ND_IF_STMT, ifStmt},          {ND_EXPR_STMT, expr},
      {ND_PRINT_STMT, print},        {ND_WHILE_STMT, whileLoop},
      {ND_FOR_STMT, forLoop},        {ND_BREAK_STMT, _break},
      {ND_CONTINUE_STMT, _continue}, {ND_STRUCT_STMT, structDecl},
      {ND_ENUM_STMT, enumDecl},      {ND_IMPORT_STMT, importDecl},
      {ND_LINK_STMT, linkFile},      {ND_EXTERN_STMT, externName},
      {ND_MATCH_STMT, matchStmt},    {ND_INPUT_STMT, inputStmt},
      {ND_CLOSE, closeStmt},
  };
  exprHandlers = {
      {ND_BINARY, binary},
      {ND_UNARY, unary},
      {ND_CALL, call},
      {ND_TERNARY, ternary},
      {ND_INT, primary},
      {ND_FLOAT, primary},
      {ND_IDENT, primary},
      {ND_STRING, primary},
      {ND_BOOL, primary},
      {ND_CHAR, primary},
      {ND_GROUP, grouping},
      {ND_ASSIGN, assign},
      {ND_PREFIX, unary},
      {ND_POSTFIX, unary},
      {ND_ARRAY, _arrayExpr},
      {ND_INDEX, arrayElem},
      {ND_CAST, cast},
      {ND_MEMBER, memberExpr},
      {ND_EXTERNAL_CALL, externalCall},
      {ND_STRUCT, _struct},
      {ND_ADDRESS, addressExpr},
      {ND_DEREFERENCE, dereferenceExpr},
      {ND_NULL, nullExpr},
      {ND_FREE_MEMORY, freeExpr},
      {ND_ALLOC_MEMORY, allocExpr},
      {ND_SIZEOF, sizeofExpr},
      {ND_MEMCPY_MEMORY, memcpyExpr},
      {ND_OPEN, openExpr},
      {ND_GETARGC, getArgcExpr},
      {ND_GETARGV, getArgvExpr},
      {ND_STRCMP, strcmp},
      {ND_COMMAND, commandExpr},
  };
  opMap = {
      {"+", "add"},   {"-", "sub"},    {"*", "imul"}, {"/", "idiv"},
      {"%", "mod"}, // mod is div but special
      {"^", "exp"},   {"~", "not"},    {"<<", "shl"}, {">>", "shr"},

      {"==", "sete"}, {"!=", "setne"}, {">", "setg"}, {">=", "setge"},
      {"<", "setl"},  {"<=", "setle"}, {"||", "lor"}, {"|", "bor"},
      {"&&", "land"},
  };
  typeSizes = {
      {"int", 8},
      {"float", 4},
      {"enum", 4},
      {"str", 8},
      {"short", 2},
      {"char", 1},
      {"bool", 1},
      {"void", 0},
      {"double", 8},
      {"$", 0}, // Imagine this represents None- an integer literal
                // whose size depends on the context.
      {"long double", 10},
      {"long", 4},
  };
}
