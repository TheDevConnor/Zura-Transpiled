#include "../common.hpp"
#include "../helper/error/error.hpp"

#include "gen.hpp"
#include "optimize.hpp"
#include "stringify.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

void codegen::handlerError(int line, int pos, std::string msg,
                               std::string note, std::string typeOfError) {
  Lexer lexer; // dummy lexer
  if (note != "")
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, false, true);
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, false, false, true);
}

bool codegen::execute_command(const std::string &command,
                              const std::string &log_file) {
  std::string command_with_error_logging = command + " 2> " + log_file;
  int result = std::system(command_with_error_logging.c_str());
  // Read log file 
  std::ifstream log(log_file);
  std::string log_contents = "";
  if (log.is_open()) {
    std::string line;
    bool first = true;
    while (getline(log, line)) {
      if (first) {
        log_contents += line;
        first = false;
      } else {
        log_contents += "\n\t" + line;
      }
    }
  }
  log.close();

  if (result != 0) {
    handlerError(0, 0, "Error executing command: " + command, log_contents,
                 "Codegen Error");
    return false;
  }
  return true;
}

void codegen::push(Instr instr, Section section) {
  if (section == Section::Main) {
    text_section.push_back(instr);
  } else if (section == Section::Head) {
    head_section.push_back(instr);
  } else if (section == Section::Data) {
    data_section.push_back(instr);
  }
}

void codegen::gen(Node::Stmt *stmt, bool isSaved, std::string output_filename,
                  const char *filename) {
  file_name = filename;
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

  std::ofstream file(output_filename + ".s");
  if (file.is_open()) {
    file << "# ┌-------------------------------┐\n"
            "# |   Zura lang by TheDevConnor   |\n"
            "# |  assembly by Soviet Pancakes  |\n"
            "# └-------------------------------┘\n"
            "# "
         << ZuraVersion
         << "\n"
            "# What's New: Loops (While/For)\n";
    // Copying gcc and hoping something changes (it won't)
    // .file directive does not like non-c and non-cpp files but it might be
    // useful for something somewhere later
    file << "#.file \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n";
    file << "\n# data section for string and stuff"
            "\n.data\n";
    file << Stringifier::stringifyInstrs(data_section);
    file << ".text\n"
            ".globl main\n"
            ".type main, @function\n"
            "main:\n"
            "  .cfi_startproc\n"
            "  pushq %rbp\n"
            "  movq %rsp, %rbp\n"
            "  call _start\n"
            "  movq %rax, %rdi\n"
            "  movq $60, %rax\n"
            "  syscall\n"
            // Technically, `ret` not needed becuase func exits above but we pretend that it is
            "  ret\n"
            "  .cfi_endproc\n"
            ".size main, .-main\n";
    file << Stringifier::stringifyInstrs(text_section);
    file << "\n# zura functions\n";
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
    file << "\n# non-main user functions" << std::endl;
    file << Stringifier::stringifyInstrs(head_section);
    file.close();
  } else {
    handlerError(0, 0,
                 "Unable to open output file for finalized assembly '" +
                     output_filename +
                     ".s' - ensure Zura has permissions to write/create files",
                 "", "Codegen Error");
  }

  output_filename =
      output_filename.substr(0, output_filename.find_last_of("."));

  ErrorClass::printError(); // Print any errors that may have occurred

  // Compile, but do not link main.o
  std::string assembler =
      "gcc -c " + output_filename + ".s -o " + output_filename + ".o";
  std::string assembler_log = output_filename + "_assembler.log";
  if (!execute_command(assembler, assembler_log))
    return;

  // Link with entry point "main"
  std::string linker =
      "ld " + output_filename + ".o -e main -o " + output_filename;
  std::string linker_log = output_filename + "_linker.log";
  if (!execute_command(linker, linker_log))
    return;

  if (!isSaved) {
    std::string remove =
        "rm " + output_filename + ".s " + output_filename + ".o";
    system(remove.c_str());
  }

  // delete the log files
  // (they are only necessary when something actually goes wrong,
  // and this code is only reached when we know it doesn't)
  std::string remove_log = "rm " + assembler_log + " " + linker_log;
  system(remove_log.c_str());
}