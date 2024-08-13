#include "../common.hpp"

#include "gen.hpp"
#include "optimize.hpp"
#include "stringify.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

void codegen::push(Instr instr, bool isSectionText) {
  if (isSectionText) {
    text_section.push_back(instr);
  } else {
    head_section.push_back(instr);
  }
}

// made "output" less ambiguous
void codegen::gen(Node::Stmt *stmt, bool isSaved, std::string output_filename) {
  initMaps();

  text_section.clear();
  head_section.clear();
  // output_code.clear();

  // stmt->debug();

  visitStmt(stmt);

  text_section = Optimizer::optimizeInstrs(text_section); 
  head_section = Optimizer::optimizeInstrs(head_section);

  std::ofstream file(output_filename + ".asm");
  if (file.is_open()) {
    file << "section .text\n";
    file << "global _start\n";
    file << Stringifier::stringifyInstrs(text_section);
    file << "\n; user-defined and Zura-native functions\n";
    file << Stringifier::stringifyInstrs(head_section);
    file.close();
  } else {
    std::cerr << "Unable to open file" << std::endl;
  }

  output_filename = output_filename.substr(0, output_filename.find_last_of("."));

  std::string assembler = "nasm -f elf64 " + output_filename + ".asm -o " + output_filename + ".o";
  const char *assembler_cstr = assembler.c_str();
  system(assembler_cstr);

  std::string linker = "ld " + output_filename + ".o -o " + output_filename;
  const char *linker_cstr = linker.c_str();
  system(linker_cstr);

  if (!isSaved) {
    std::string remove = "rm " + output_filename + ".asm " + output_filename + ".o";
    system(remove.c_str());
  }
}
