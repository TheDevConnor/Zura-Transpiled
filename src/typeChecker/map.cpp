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
        {NodeKind::ND_MATCH_STMT, visitMatch},
        {NodeKind::ND_INPUT_STMT, visitInput},
        {NodeKind::ND_CLOSE, visitClose}
};

std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>
    TypeChecker::exprs = {
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
        {NodeKind::ND_FREE_MEMORY, visitFreeMemory},
        {NodeKind::ND_ALLOC_MEMORY, visitAllocMemory},
        {NodeKind::ND_SIZEOF, visitSizeof},
        {NodeKind::ND_MEMCPY_MEMORY, visitMemcpyMemory},
        {NodeKind::ND_OPEN, visitOpen}
};

Node::Stmt *TypeChecker::StmtAstLookup(Node::Stmt *node, Maps *maps) {
  std::vector<std::pair<NodeKind, TypeChecker::StmtNodeHandler>>::iterator res = std::find_if(stmts.begin(), stmts.end(), [&](std::pair<NodeKind, TypeChecker::StmtNodeHandler> &stmtHandler) {
    return node->kind == stmtHandler.first;
  });
  if (res != stmts.end())
    res->second(maps, node);
  return node;
}

Node::Expr *TypeChecker::ExprAstLookup(Node::Expr *node, Maps *maps) {
  std::vector<std::pair<NodeKind, TypeChecker::ExprNodeHandler>>::iterator res = std::find_if(exprs.begin(), exprs.end(), [&](std::pair<NodeKind, TypeChecker::ExprNodeHandler> &exprHandler) {
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
  TypeChecker::FnVector::iterator res = std::find_if(
      maps->function_table.begin(), maps->function_table.end(),
      [&pair](const Fn &fn) { return fn.first.first == pair.first; });

  if (res != maps->function_table.end()) {
    std::string msg = "Function '" + pair.first + "' is already defined";
    handleError(line, pos, msg, "", "Type Error");
  }

  if (pair.first == "main") {
    if (foundMain) {
      std::string msg = "Entry (main) function is already defined";
      handleError(line, pos, msg, "", "Type Error");
    }
    if (type_to_string(pair.second) != "int") {
      std::string msg = "Main function must return an uint";
      handleError(line, pos, msg, "", "Type Error");
    }
    if (paramTypes.size() != 0) {
      std::string msg = "Main function must not have any parameters";
      handleError(line, pos, msg, "", "Type Error");
    }
    maps->function_table.push_back({pair, paramTypes});
    foundMain = true;
    return;
  }

  // add the function to the function table
  maps->function_table.push_back({pair, paramTypes});
}

void TypeChecker::declare_struct_fn(Maps *maps, const std::pair<std::string, Node::Type *> &pair,
                        std::vector<std::pair<std::string, Node::Type *>> paramTypes,
                        int line, int pos, std::string structName) {
  // check if the function is already defined in the struct_table_fn
  for (std::pair<std::pair<std::string, Node::Type *>,
        std::vector<std::pair<std::string, Node::Type *>>> member : maps->struct_table_fn[structName]) {
    if (member.first.first == pair.first) {
      std::string msg = "Function '" + pair.first + "' is already defined in struct '" + structName + "'";
      handleError(line, pos, msg, "", "Type Error");
    }
  }

  // add the function to the struct_table_fn
  maps->struct_table_fn[structName].push_back({pair, paramTypes});
}

FnVector TypeChecker::lookup_fn(Maps *maps, Node::Expr *callee, int line, int pos) {
  if (callee->kind == ND_IDENT) {
    std::string name = static_cast<IdentExpr *>(callee)->name;
    FnVector::iterator res =
        std::find_if(maps->function_table.begin(), maps->function_table.end(),
                    [&name](const Fn &fn) { return fn.first.first == name; });

    if (res != maps->function_table.end()) {
      return { *res };
    }

    std::string msg = "Function '" + name + "' is not defined";
    handleError(line, pos, msg, "", "Type Error");
  } else if (callee->kind == ND_MEMBER) {
    MemberExpr *member = static_cast<MemberExpr *>(callee);
    // visit the lhs of the member expression
    visitExpr(maps, member->lhs);
    std::string lhsType = type_to_string(return_type.get());
    // get the name of the member
    std::string name = static_cast<IdentExpr *>(member->rhs)->name;
    // only structs can have function members
    if (maps->struct_table.find(lhsType) == maps->struct_table.end()) {
      std::string msg = "Function '" + name + "' is not defined in struct '" + lhsType + "'";
      handleError(member->line, member->pos, msg, "", "Type Error");
    }
    // find the function in the struct_table_fn
    for (std::pair<std::pair<std::string, Node::Type *>,
          std::vector<std::pair<std::string, Node::Type *>>> member : maps->struct_table_fn[lhsType]) {
      if (member.first.first == name) {
        return { member };
      }
    }
    std::string msg = "Function '" + name + "' is not defined in struct '" + lhsType + "'";
    handleError(member->line, member->pos, msg, "", "Type Error");
  }
  return {};
}

void TypeChecker::printTables(Maps *map) {
  std::cout << "Global Table" << std::endl;
  for (std::pair<const std::string, Node::Type *> &pair : map->global_symbol_table) {
    std::cout << "\t" << pair.first << " : " << type_to_string(pair.second)
              << std::endl;
  }

  std::cout << "Local Table" << std::endl;
  for (std::pair<const std::string, Node::Type *> &pair : map->local_symbol_table) {
    std::cout << "\t" << pair.first << " : " << type_to_string(pair.second)
              << std::endl;
  }

  std::cout << "Function Table" << std::endl;
  for (Fn &pair : map->function_table) {
    std::cout << "\t" << pair.first.first << " : "
              << type_to_string(pair.first.second) << std::endl;
    for (std::pair<std::string, Node::Type *> &param : pair.second) {
      std::cout << "\t\t" << param.first << " : "
                << type_to_string(param.second) << std::endl;
    }
  }

  std::cout << "Array Table" << std::endl;
  for (Node::Type *type : map->array_table) {
    std::cout << "\t" << type_to_string(type) << std::endl;
  }

  std::cout << "Struct Table" << std::endl;
  for (std::pair<std::string, std::vector<std::pair<std::string, Node::Type *>>> pair : map->struct_table) {
    std::cout << "\t" << pair.first << " : " << std::endl;
    for (std::pair<std::string, Node::Type *> member : pair.second) {
      std::cout << "\t\t" << member.first << " : "
                << type_to_string(member.second) << std::endl;
    }
  }

  std::cout << "Struct Function Table" << std::endl;
  for (std::pair<std::string, std::vector<std::pair<std::pair<std::string, Node::Type *>, std::vector<std::pair<std::string, Node::Type *>>>>> pair : map->struct_table_fn) {
    std::cout << "\t" << pair.first << " : " << std::endl;
    for (std::pair<std::pair<std::string, Node::Type *>, std::vector<std::pair<std::string, Node::Type *>>> member : pair.second) {
      std::cout << "\t\t" << member.first.first << " : "
                << type_to_string(member.first.second) << std::endl;
      for (std::pair<std::string, Node::Type *> param : member.second) {
        std::cout << "\t\t\t" << param.first << " : "
                  << type_to_string(param.second) << std::endl;
      }
    }
  }

  std::cout << "Enum Table" << std::endl;
  for (std::pair<std::string, std::vector<std::pair<std::string, int>>> pair : map->enum_table) {
    std::cout << "\t" << pair.first << " : " << std::endl;
    for (std::pair<std::string, int> &member : pair.second) {
      std::cout << "\t\t" << member.first << " : " << member.second
                << std::endl;
    }
  }

  std::cout << "Template Table" << std::endl;
  for (std::string &name : map->template_table) {
    std::cout << "\t" << name << std::endl;
  }
}