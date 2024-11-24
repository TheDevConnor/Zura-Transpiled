#include "../ast/ast.hpp"
#include "type.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace TypeChecker;

std::vector<std::pair<NodeKind, TypeChecker::StmtNodeHandler>>
    TypeChecker::stmts = {
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
        {NodeKind::ND_TEMPLATE_STMT, visitTemplateStmt},
        {NodeKind::ND_WHILE_STMT, visitWhile},
        {NodeKind::ND_FOR_STMT, visitFor},
        {NodeKind::ND_BREAK_STMT, visitBreak},
        {NodeKind::ND_CONTINUE_STMT, visitContinue},
        {NodeKind::ND_IMPORT_STMT, visitImport},
        {NodeKind::ND_LINK_STMT, visitLink},
        {NodeKind::ND_EXTERN_STMT, visitExtern},
};

std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>
    TypeChecker::exprs = {
        {NodeKind::ND_INT, visitInt},
        {NodeKind::ND_FLOAT, visitFloat},
        {NodeKind::ND_IDENT, visitIdent},
        {NodeKind::ND_STRING, visitString},
        {NodeKind::ND_BINARY, visitBinary},
        {NodeKind::ND_CALL, visitCall},
        {NodeKind::ND_TERNARY, visitTernary},
        {NodeKind::ND_GROUP, visitGrouping},
        {NodeKind::ND_UNARY, visitUnary},
        {NodeKind::ND_MEMBER, visitMember},
        {NodeKind::ND_ASSIGN, visitAssign},
        {NodeKind::ND_ARRAY, visitArray},
        {NodeKind::ND_INDEX, visitIndex},
        {NodeKind::ND_PREFIX, visitUnary},
        {NodeKind::ND_POSTFIX, visitUnary},
        {NodeKind::ND_CAST, visitCast},
        {NodeKind::ND_BOOL, visitBool},
        {NodeKind::ND_TEMPLATE_CALL, visitTemplateCall},
        {NodeKind::ND_EXTERNAL_CALL, visitExternalCall},
};

Node::Stmt *TypeChecker::StmtAstLookup(Node::Stmt *node, Maps *maps) {
  auto res = std::find_if(stmts.begin(), stmts.end(), [&](auto &stmtHandler) {
    return node->kind == stmtHandler.first;
  });
  if (res != stmts.end())
    res->second(maps, node);
  return node;
}

Node::Expr *TypeChecker::ExprAstLookup(Node::Expr *node, Maps *maps) {
  auto res = std::find_if(exprs.begin(), exprs.end(), [&](auto &exprHandler) {
    return node->kind == exprHandler.first;
  });
  if (res != exprs.end())
    res->second(maps, node);
  return node;
}

void TypeChecker::declare_fn(Maps *maps, const std::pair<std::string, Node::Type *> &pair,
                std::vector<std::pair<std::string, Node::Type *>> paramTypes,
                int line, int pos) {

  // check if the function is already defined
  auto res = std::find_if(
      maps->function_table.begin(), maps->function_table.end(),
      [&pair](const auto &fn) { return fn.first.first == pair.first; });

  if (res != maps->function_table.end()) {
    std::string msg = "Function '" + pair.first + "' is already defined";
    handlerError(line, pos, msg, "", "Symbol Table Error");
  }

  if (pair.first == "main") {
    if (foundMain) {
      std::string msg = "Main function is already defined";
      handlerError(line, pos, msg, "", "Symbol Table Error");
    }
    if (type_to_string(pair.second) != "int") {
      std::string msg = "Main function must return an int";
      handlerError(line, pos, msg, "", "Symbol Table Error");
    }
    if (paramTypes.size() != 0) {
      std::string msg = "Main function must not have any parameters";
      handlerError(line, pos, msg, "", "Symbol Table Error");
    }
    maps->function_table.push_back({pair, paramTypes});
    foundMain = true;
    return;
  }

  // add the function to the function table
  maps->function_table.push_back({pair, paramTypes});
}

std::vector<std::pair<std::pair<std::string, Node::Type *>,
                      std::vector<std::pair<std::string, Node::Type *>>>>
TypeChecker::lookup_fn(Maps *maps, std::string name, int line, int pos) {
  auto res =
      std::find_if(maps->function_table.begin(), maps->function_table.end(),
                   [&name](const auto &fn) { return fn.first.first == name; });

  if (res != maps->function_table.end()) {
    return { *res };
  }

  std::string msg = "Function '" + name + "' is not defined";
  handlerError(line, pos, msg, "", "Symbol Table Error");
  return {};
}

void TypeChecker::printTables(Maps *map) {
  std::cout << "Global Table" << std::endl;
  for (auto &pair : map->global_symbol_table) {
    std::cout << "\t" << pair.first << " : " << type_to_string(pair.second)
              << std::endl;
  }

  std::cout << "Local Table" << std::endl;
  for (auto &pair : map->local_symbol_table) {
    std::cout << "\t" << pair.first << " : " << type_to_string(pair.second)
              << std::endl;
  }

  std::cout << "Function Table" << std::endl;
  for (auto &pair : map->function_table) {
    std::cout << "\t" << pair.first.first << " : "
              << type_to_string(pair.first.second) << std::endl;
    for (auto &param : pair.second) {
      std::cout << "\t\t" << param.first << " : "
                << type_to_string(param.second) << std::endl;
    }
  }

  std::cout << "Array Table" << std::endl;
  for (auto &type : map->array_table) {
    std::cout << "\t" << type_to_string(type) << std::endl;
  }

  std::cout << "Struct Table" << std::endl;
  for (auto &pair : map->struct_table) {
    std::cout << "\t" << pair.first << " : " << std::endl;
    for (auto &member : pair.second) {
      std::cout << "\t\t" << member.first << " : "
                << type_to_string(member.second) << std::endl;
    }
  }

  std::cout << "Enum Table" << std::endl;
  for (auto &pair : map->enum_table) {
    std::cout << "\t" << pair.first << " : " << std::endl;
    for (auto &member : pair.second) {
      std::cout << "\t\t" << member.first << " : " << member.second
                << std::endl;
    }
  }
}