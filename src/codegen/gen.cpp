#include "../common.hpp"

#include "gen.hpp"
#include "optimize.hpp"
#include "stringify.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

void codegen::push(Instr instr, Section section) {
  if (section == Section::Main) {
    text_section.push_back(instr);
  } else if (section == Section::Head) {
    head_section.push_back(instr);
  } else if (section == Section::Data) {
    data_section.push_back(instr);
  }
}

void codegen::gen(Node::Stmt *stmt, bool isSaved, std::string output_filename) {
  initMaps();

  text_section.clear();
  head_section.clear();
  data_section.clear();
  // output_code.clear();

  // stmt->debug();

  visitStmt(stmt);

  text_section = Optimizer::optimizeInstrs(text_section); 
  head_section = Optimizer::optimizeInstrs(head_section);
  // data section cannot be optimized

  std::ofstream file(output_filename + ".asm");
  if (file.is_open()) {
    file << "; ---------------------------------\n"; 
    file << "; -   Zura lang by TheDevConnor   -\n";
    file << "; - asm helped by Soviet Pancakes -\n";
    file << "; ---------------------------------\n";
    file << "; What's new: Return If Statements, String/Print Fix\n\n";
    file << "BITS 64\n";
    file << "section .text\n";
    file << "global _start\n";
    file << Stringifier::stringifyInstrs(text_section);
    file << "\n; zura functions\n";
    if (nativeFunctionsUsed[NativeASMFunc::strlen] == true) {
      file << "\nnative_strlen:"
              "\n  push  rbx                 ; save any registers that "
              "\n  push  rcx                 ; we will trash in here"
              "\n  mov   rbx, rdi            ; rbx = rdi"
              "\n  xor   al, al              ; look for NUL term (0x0)"
              "\n  mov   rcx, 0xffffffff     ; the max string length is 4gb"
              "\n  repne scasb               ; while [rdi] != al && rcx > 0, rdi++"
              "\n  sub   rdi, rbx            ; length = end - start"
              "\n  mov   rax, rdi            ; rax now holds our length"
              "\n  pop   rcx                 ; restore the saved registers"
              "\n  pop   rbx"
              "\n  ret\n";
    }
    file << "\n; non-main user functions" << std::endl;
    file << Stringifier::stringifyInstrs(head_section);
    file << "\n; data section for string and stuff";
    file << "\nsection .data:\n";
    file << Stringifier::stringifyInstrs(data_section);
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