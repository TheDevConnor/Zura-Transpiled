#include "type.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"
#include <memory>

void TypeChecker::performCheck(Node::Stmt *stmt, bool isMain, bool isLspServer) {
  // Reset everything before we check
  foundMain = false;
  needsReturn = false;
  return_type = nullptr;
  lsp_idents.clear();
  
  // std::unique_ptr<Maps> map = std::make_unique<Maps>();
  Maps *map = new Maps();
  isLspMode = isLspServer;

  visitStmt(map, stmt); // Pass the instance of Maps to visitStmt 
  if (!foundMain && isMain) {
    handleError(0, 0, "No main function found",
                 "Try adding this function: \n\tconst main := fn() int { \n\t  "
                 "  return 0\n\t}",
                 "Type Error");
  }

  delete map;
}