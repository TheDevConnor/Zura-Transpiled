#include <cstring>
#include <fstream>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "gen.hpp"

void Gen::functionDeclaration(std::ofstream &file, AstNode *node) {
  AstNode::FunctionDeclaration *fun =
      static_cast<AstNode::FunctionDeclaration *>(node->data);

  const char *name = fun->name.start;
  name = strtok(const_cast<char *>(name), "(");
  file << "\n" << name << ":\n";

  prologue(file);

  blockStmt(file, fun->body);

  epilogue(file);
}
