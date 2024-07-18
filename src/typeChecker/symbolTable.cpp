#include "../ast/ast.hpp"
#include "../ast/types.hpp"
#include "../helper/error/error.hpp"
#include "type.hpp"

#include <string>
#include <vector>

/// Symbol Table function to push a variable into the symbol table
void TypeChecker::declare(symbol_table &table, std::string name,
                          Node::Type *type) {
  table[name] = type;
}

Node::Type *TypeChecker::table_lookup(symbol_table &table, std::string name, int line, int pos) {
  Lexer lexer; // dummy lexer
  for (auto &pair : table) {
    if (pair.first == name) {
      return pair.second;
    }
  }
  std::string msg = "Undeclared variable '" + name + "'";
  handlerError(line, pos, msg);
  return nullptr;
}

void TypeChecker::declare(
    callables_table &table, std::string name,
    std::vector<std::pair<std::string, Node::Type *>> params) {
  table[name] = params;
}

std::vector<std::pair<std::string, Node::Type *>>
TypeChecker::table_lookup(callables_table &table, std::string name, int line, int pos) {
  for (auto &pair : table) {
    if (pair.first == name) {
      return pair.second;
    }
  }
  std::string msg = "Undeclared function '" + name + "'";
  handlerError(line, pos, msg);
  return {};
}