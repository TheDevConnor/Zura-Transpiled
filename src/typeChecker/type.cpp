#include "type.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"
#include <memory>

void TypeChecker::performCheck(Node::Stmt *stmt, bool isMain) {
  std::unique_ptr<Maps> map = std::make_unique<Maps>();
  visitStmt(map.get(), stmt); // Pass the instance of Maps to visitStmt

  if (!foundMain && isMain) {
    std::cout << "No main function found" << std::endl;
    handleError(0, 0, "No main function found",
                 "Try adding this function: \n\tconst main := fn() int { \n\t  "
                 "  return 0\n\t}",
                 "Type Error");
  }
}

