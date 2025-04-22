#include "../helper/error/error.hpp"
#include "../ast/stmt.hpp"
#include "typeMaps.hpp"
#include "type.hpp"

#include <memory>
#include <filesystem>

void TypeChecker::performCheck(Node::Stmt *stmt, bool isMain, bool isLspServer) {
  isLspMode = isLspServer;
  // Clear lsp idents that are in the current file
  for (auto it = lsp_idents.begin(); it != lsp_idents.end();) {
    if (std::filesystem::path(static_cast<ProgramStmt *>(stmt)->inputPath).relative_path().string() == node.current_file) {
      it = lsp_idents.erase(it);
    } else {
      ++it;
    }
  }

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