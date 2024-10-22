#include "../helper/error/error.hpp"
#include "../common.hpp"

#include "gen.hpp"
#include "optimizer/optimize.hpp"
#include "optimizer/stringify.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

void codegen::gen(Node::Stmt *stmt, bool isSaved, std::string output_filename,
                  const char *filename, bool isDebug) {
  file_name = filename;
  debug = isDebug;
  initMaps();

  text_section.clear();
  head_section.clear();
  data_section.clear();
  rodt_section.clear();
  // output_code.clear();

  // stmt->debug();

  visitStmt(stmt);

  // Make 2 passes of optimization
  text_section = Optimizer::optimizeInstrs(text_section);
  text_section = Optimizer::optimizeInstrs(text_section);
  
  head_section = Optimizer::optimizeInstrs(head_section);
  head_section = Optimizer::optimizeInstrs(head_section);
  // data section cannot be optimized
  // rodata section cant be optimized either

  std::ofstream file(output_filename + ".s");
  if (!file.is_open()) handlerError(0, 0,
              "Unable to open output file for finalized assembly '" +
                  output_filename +
                  ".s' - ensure Zura has permissions to write/create files",
              "", "Codegen Error");
  
  file << "# ┌-------------------------------┐\n"
          "# |   Zura lang by TheDevConnor   |\n"
          "# |  assembly by Soviet Pancakes  |\n"
          "# └-------------------------------┘\n"
          "# "
        << ZuraVersion
        << "\n"
          "# What's New: Debug symbols (Open GDB and try it!)\n";
  // This one defines the file the whole assembly is related to
  if (debug) {
    file << ".file   \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n";
    file << ".file 0 \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n";
  }

  if (data_section.size() > 0) {
    file << "\n# data section for pre-allocated, mutable data"
            "\n.data\n";
    file << Stringifier::stringifyInstrs(data_section);
  }
  if (rodt_section.size() > 0) {
    file << "\n# readonly data section - contains constant strings and floats (for now)"
            "\n.section .rodata\n";
    file << Stringifier::stringifyInstrs(rodt_section);
  }
  file << ".text\n"
          ".globl main\n";
  file << Stringifier::stringifyInstrs(text_section);
  if (nativeFunctionsUsed.size() > 0) file << "\n# zura functions\n";
  if (nativeFunctionsUsed[NativeASMFunc::strlen] == true) {
    file << ".type native_strlen, @function\n"
            "native_strlen:\n"
            "  pushq %rbp              # save base pointer\n"
            "  movq %rsp, %rbp         # set base pointer\n"
            "  movq %rdi, %rcx         # move string to rcx\n"
            "  xorq %rax, %rax         # clear rax\n"
            "  strlen_loop:\n"
            "    cmpb $0, (%rcx, %rax) # compare byte at rcx + rax to 0\n"
            "    je strlen_end         # if byte is 0, end\n"
            "    incq %rax             # increment rax\n"
            "    jmp strlen_loop # loop\n"
            "  strlen_end:\n"
            "  popq %rbp               # restore base pointer\n"
            "  ret # return\n"
            ".size native_strlen, .-native_strlen\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::itoa] == true) {
    file << ".type native_itoa, @function\n"
          "native_itoa:\n"
          "  pushq %rbp              # save base pointer\n"
          "  movq %rsp, %rbp         # set base pointer\n"
          "  movq $-1, %rdi          # buffer\n"
          "  leaq (%rsp, %rdi, 1), %rbx\n"
          "  movq %rsi, %rax         # number\n"
          "  movq $0, %rdi           # counter\n"
          "  pushq $0x0\n"
          "  jmp itoa_loop\n"
          "itoa_loop:\n"
          "  movq $0, %rdx\n"
          "  movq $10, %rcx\n"
          "  div %rcx\n"
          "  addq $48, %rdx\n"
          "  pushq %rdx\n"
          "  movb (%rsp), %cl\n"
          "  movb %cl, (%rbx, %rdi, 1)\n"
          "  test %rax, %rax\n"
          "  je itoa_end\n"
          "  dec %rdi\n"
          "  jmp itoa_loop\n"
          "itoa_end:\n"
          "  leaq (%rbx, %rdi, 1), %rax\n"
          "  movq %rbp, %rsp\n"
          "  popq %rbp\n"
          "  ret\n"
          ".size native_itoa, .-native_itoa\n";
  }
  if (head_section.size() > 0) {
    file << "\n# non-main user functions" << std::endl;
    file << Stringifier::stringifyInstrs(head_section);
  }
  file.close();

  output_filename =
      output_filename.substr(0, output_filename.find_last_of("."));

  ErrorClass::printError(); // Print any errors that may have occurred

  // Compile, but do not link main.o
  std::string assembler =
      "gcc -c -g " + output_filename + ".s -o " + output_filename + ".o";
  std::string assembler_log = output_filename + "_assembler.log";
  if (!execute_command(assembler, assembler_log))
    return;

  // Link with entry point "main"
  std::string linker =
      "ld " + output_filename + ".o -e main -o " + output_filename;
  std::string linker_log = output_filename + "_linker.log";
  if (!execute_command(linker, linker_log))
    return;
  int exitCode; // NOTE: This is to remove warnings. It is not practical lmao
  if (!isSaved) {
    std::string remove =
        "rm " + output_filename + ".s " + output_filename + ".o";
    exitCode = system(remove.c_str());
  }

  // delete the log files
  // (they are only necessary when something actually goes wrong,
  // and this code is only reached when we know it doesn't)
  std::string remove_log = "rm " + assembler_log + " " + linker_log;
  exitCode = system(remove_log.c_str());
}