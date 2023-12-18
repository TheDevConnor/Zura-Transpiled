#include "gen.hpp"
#include <fstream>

Gen::Gen(AstNode *ast) : ast(ast) { generate(); }

void Gen::generate() {
  std::ofstream file;
  file.open("out.c");

  file << "// -----------------------------------------\n";
  file << "// Compile Zura to C\n";
  file << "// -----------------------------------------\n\n";

  headerImport(file);

  body(file, ast);
}
