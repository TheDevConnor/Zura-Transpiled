#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "typeMaps.hpp"
#include "type.hpp"

#include <memory>

void TypeChecker::performCheck(Node::Stmt *stmt, bool isMain, bool isLspServer) {
  isLspMode = isLspServer;
  lsp_idents.clear();

  initMaps(); // Initialize the maps

  if (!context) {
    context = std::make_unique<TypeCheckerContext>();
  }

  context->enterScope();
  visitStmt(stmt); // Pass the instance of Maps to the visitor
  context->exitScope();

  if (!foundMain && isMain) {
    std::string msg = "No main function found";
    std::string note = "Try adding this function: \n\tconst main := fn() int { \n\t  return 0\n\t}";
    handleError(0, 0, msg, note, "Type Error");
  }
}