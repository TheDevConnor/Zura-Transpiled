#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

#include "../lexer/lexer.hpp"
#include "gen.hpp"

Gen::Gen(AstNode *ast) : ast(ast) { generate(); }

void Gen::generate() {
  std::ofstream file;
  file.open("out.c");

  file << "// -----------------------------------------\n";
  file << "// Compile Zura to C\n";
  file << "// -----------------------------------------\n\n";

  headerImport(file);

  if (ast->type == AstNodeType::PROGRAM) {
    AstNode::Program *program = (AstNode::Program *)ast->data;

    for (AstNode *node : program->statements) {
      if (node->type == AstNodeType::FUNCTION_DECLARATION) {
        AstNode::FunctionDeclaration *function =
            (AstNode::FunctionDeclaration *)node->data;
        functionDeclaration(file, function);
      }
    }
  }
}
