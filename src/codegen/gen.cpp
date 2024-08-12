#include "../common.hpp"

#include "gen.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

void codegen::push(std::string str, bool isSectionText) {
  if (isSectionText) {
    output_code += str + "\n";
    section_text.push_back(str);
  } else {
    output_code += str + "\n";
    section_data.push_back(str);
  }
}

void codegen::gen(Node::Stmt *stmt, bool isSaved, std::string output) {
  initMaps();

  section_data.clear();
  section_text.clear();
  output_code.clear();

  stmt->debug();

  visitStmt(stmt);

  std::ofstream file(output + ".asm");
  if (file.is_open()) {
    file << "section .text\n";
    file << "global _start\n";
    for (const auto &line : section_text) {
      file << line << "\n";
    }
    file.close();
  } else {
    std::cerr << "Unable to open file" << std::endl;
  }
  output_code.clear();

  output = output.substr(0, output.find_last_of("."));

  std::string assembler = "nasm -f elf64 " + output + ".asm -o " + output + ".o";
  const char *assembler_cstr = assembler.c_str();
  system(assembler_cstr);

  std::string linker = "ld " + output + ".o -o " + output;
  const char *linker_cstr = linker.c_str();
  system(linker_cstr);

  if (!isSaved) {
    std::string remove = "rm " + output + ".asm " + output + ".o";
    system(remove.c_str());
  }
}
