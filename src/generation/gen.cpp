#include <fstream>
#include <memory>

#include "../lexer/lexer.hpp"
#include "gen.hpp"

Gen::Gen(std::unique_ptr<AstNode> ast) : ast(std::move(ast)) { generate(); }

void Gen::generate() {
  std::ofstream file;
  file.open("out.c");

  file << "// -----------------------------------------\n";
  file << "// Compile Zura to C\n";
  file << "// -----------------------------------------\n\n";

  headerImport(file);

  body(file, ast.get());
}
