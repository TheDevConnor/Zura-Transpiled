#include "gen.hpp"
#include <fstream>

Gen::Gen(AstNode *ast) : ast(ast) { generate(); }

void Gen::generate() {
  std::ofstream file;
  file.open("out.asm");

  file << "; -----------------------------------------\n";
  file << "; Compile Zura to Asm\n";
  file << "; -----------------------------------------\n\n";

  body(file, ast);
}
