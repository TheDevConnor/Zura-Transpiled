#include "../ast/ast.hpp"
#include "../ast/types.hpp"
#include "../helper/error/error.hpp"
#include "type.hpp"

#include <string>
#include <vector>

void TypeChecker::declare_fn(
    function_table &fn_table, std::string name, const NameTypePair &pair,
    std::vector<std::pair<std::string, Node::Type *>> paramTypes, int line,
    int pos) {
  fn_table.push_back({pair, paramTypes});
}