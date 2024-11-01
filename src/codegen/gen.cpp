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
                  ".s' - ensure Zura has permissions to write/create files", "Codegen Error");
  
  file << "# ┌-------------------------------┐\n"
          "# |   Zura lang by TheDevConnor   |\n"
          "# |  assembly by Soviet Pancakes  |\n"
          "# └-------------------------------┘\n"
          "# "
        << ZuraVersion
        << "\n"
          "# What's New: Debug symbols (Open GDB and try it!)\n";
  // This one defines the file the whole assembly is related to
  if (debug)
    file << ".file   \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n";
  file << ".text\n";
  if (debug)
    file << ".Ltext0:\n.file 0 \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n";

  file << ".globl main\n";
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
  if (rodt_section.size() > 0) {
    file << "\n# readonly data section - contains constant strings and floats (for now)"
            "\n.section .rodata\n";
    file << Stringifier::stringifyInstrs(rodt_section);
  }
  if (data_section.size() > 0) {
    file << "\n# data section for pre-allocated, mutable data"
            "\n.data\n";
    file << Stringifier::stringifyInstrs(data_section);
  }

  // DWARF debug yayy
  if (debug) {
    // Debug symbols should be included
    file << ".Ldebug_text0:\n";
	  file << ".section	.debug_info,\"\",@progbits\n\t";
    file << "\n.long .Ldebug_end - .Ldebug_info" // Length of the debug info header 
            "\n.Ldebug_info:" // NOTE: ^^^^ This long tag right here requires the EXCLUSION of those 4 bytes there
            "\n.word 0x5" // DWARF version 5
            "\n.byte 0x1" // Unit-type DW_UT_compile
            "\n.byte 0x8" // 8-bytes registers (64-bit os)
            "\n.long .Ldebug_abbrev"
            "\n"
            "\n.uleb128 0x1" // Compilation unit name
            "\n.long .Ldebug_producer_string"
            "\n.byte 0x1d" // Language (C)
            "\n.long .Ldebug_file_string"
            "\n.long .Ldebug_file_dir"
            "\n.quad .Ltext0" // Low PC (beginning of .text)
            "\n.quad .Ldebug_text0-.Ltext0" // High PC (end of .text)
            "\n.long .Ldebug_line0"
            "\n";
    // Attributes or whatever that follow
    file << Stringifier::stringifyInstrs(die_section) << "\n";
    file << ".Lint_debug_type:\n"
            ".uleb128 0x3\n"
            ".long .Lint_debug_string\n"
            ".byte 0x05\n"
            ".byte 0x08\n";
    file << ".Lfloat_debug_type:\n"
            ".uleb128 0x3\n"
            ".long .Lfloat_debug_string\n"
            ".byte 0x04\n"
            ".byte 0x04\n";
    file << ".Ldebug_end:\n";
    file << ".section .debug_abbrev,\"\",@progbits\n";
    file << ".Ldebug_abbrev:\n";
        // Define compilation unit (DW_TAG_compile_unit)
    file << ".uleb128 0x1\n" // Opcode 1
            ".uleb128 0x11\n" // DW_TAG_compile_unit
            ".byte	0x1\n" // Yes children
            ".uleb128 0x25\n" // producer
            ".uleb128 0x0E\n" // strp

            ".uleb128 0x13\n" // language
            ".uleb128 0x0B\n" // byte

            ".uleb128 0x03\n" // at_name (filename)
            ".uleb128 0x1F\n" // lines_str //

            ".uleb128 0x1B\n" // comp_dir (file dir)
            ".uleb128 0x1F\n" // lines_str

            ".uleb128 0x11\n" // low pc
            ".uleb128 0x01\n" // addr

            ".uleb128 0x12\n" // high pc
            ".uleb128 0x07\n" // data 8 (quad)

            ".uleb128 0x10\n" // stmt_list
            ".uleb128 0x17\n" // sec_offset
            ".byte 0x0\n"
            ".byte 0x0\n";
    // Define Variable Declaration (DW_TAG_variable)
    file << ".uleb128 0x2\n" // Use opcode 2
            ".uleb128 0x34\n" // DW_TAG_variable
            ".byte 0\n" // No children

            ".uleb128 0x3\n" // DW_AT_name
            ".uleb128 0x0E\n" // DW_FORM_string

            ".uleb128 0x3A\n" // DW_AT_decl_file
            ".uleb128 0x0B\n" // DW_FORM_data1

            ".uleb128 0x3B\n" // DW_AT_decl_line
            ".uleb128 0x0B\n" // DW_FORM_data1

            ".uleb128 0x49\n" // DW_AT_type
            ".uleb128 0x13\n" // DW_FORM_ref4 - 4-byte pointer to the .debug_info type

            ".uleb128 0x02\n" // DW_AT_location
            ".uleb128 0x18\n" // DW_FORM_exprloc

            ".byte 0x0\n"
            ".byte 0x0\n";
    // Define Type Declaration (DW_TAG_base_type)
    file << ".Ldata_type:\n"
            ".uleb128 0x03\n" // Opcode of this abbreviation - 3
            ".uleb128 0x24\n" // DW_TAG_base_type (basic type like int, float- perfect for ASMType)
            ".byte 0x0\n" // No children

            ".uleb128 0x03\n" // DW_AT_name
            ".uleb128 0x0E\n" // DW_FORM_strp

            ".uleb128 0x3E\n" // DW_AT_encoding
            ".uleb128 0x0B\n" // DW_FORM_data1

            ".uleb128 0x0B\n" // DW_AT_byte_size
            ".uleb128 0x0B\n" // 1-byte constant that follows
            ".byte 0x0\n"
            ".byte 0x0\n"; // It's done!
    // Define Subprogram (function) Declaration (DW_TAG_subprogram)
    file << ".uleb128 0x04\n" // Use opcode 4
            ".uleb128 0x2E\n" // DW_TAG_subprogram
            ".byte 0x1\n" // Yes children :(
            
            ".uleb128 0x3F\n" // DW_AT_external
            ".uleb128 0x0C\n" // DW_FORM_flag

            ".uleb128 0x03\n" // DW_AT_name
            ".uleb128 0x0E\n" // DW_FORM_strp

            ".uleb128 0x3A\n" // DW_AT_decl_file
            ".uleb128 0x0B\n" // DW_FORM_data1

            ".uleb128 0x3B\n" // DW_AT_decl_line
            ".uleb128 0x0B\n" // DW_FORM_data1

            ".uleb128 0x49\n" // DW_AT_type
            ".uleb128 0x13\n" // DW_FORM_ref4 - 4-byte pointer to the .debug_info type

            ".uleb128 0x11\n" // DW_AT_low_pc
            ".uleb128 0x01\n" // DW_FORM_addr

            ".uleb128 0x12\n" // DW_AT_high_pc
            ".uleb128 0x07\n" // DW_FORM_data8 (?)

            ".uleb128 0x40\n" // DW_AT_frame_base
            ".uleb128 0x18\n" // DW_FORM_exprloc

            ".uleb128 0x7A\n" // DW_AT_call_all_calls
            ".uleb128 0x19\n" // DW_FORM_flag_present

            // Should I use siblings? I don't know
            // ".uleb128 0x01\n" // DW_AT_sibling
            // ".uleb128 0x13\n" // DW_FORM_ref4

            ".byte 0x0\n"
            ".byte 0x0\n";
    
    file << Stringifier::stringifyInstrs(diea_section);
    file << "\n.byte 00\n"; // End of abbreviations
    
    // Still not done
    // Aranges
    file << ".section .debug_aranges,\"\",@progbits\n";
    file << ".Ldebug_aranges:\n"
            ".long .Ldebug_aranges_end - 4 - .Ldebug_aranges\n" // length of the aranges
            ".value 0x5\n" // DWARF ver (5)
            ".long .Ldebug_text0\n"
            ".byte 0x8\n" // 8-byte addresses
            ".byte 0x0\n" // x86-64 typically has "flat" (0x0) memory model
            ".value 0\n"
            ".value 0\n"
            ".quad .Ltext0\n"
            ".quad .Ldebug_text0-.Ltext0\n"
            ".quad 0\n"
            ".quad 0\n" // End of aranges
            ".Ldebug_aranges_end:\n";

    // debug_line
    file << ".section .debug_line,\"\",@progbits\n";
    file << ".Ldebug_line0:\n";
    // TODO
    // debug_str
    file << ".section .debug_str,\"MS\",@progbits,1\n"
            ".Ldebug_producer_string: .string \"Zura compiler version " + ZuraVersion + ", debug on\"\n"
            // TOOD: Convert to path
            ".Lint_debug_string: .string \"int\"\n"
            ".Lfloat_debug_string: .string \"float\"\n";
    file << Stringifier::stringifyInstrs(dies_section) << "\n";
    // debug_line_str
    std::string fileRelPath = static_cast<ProgramStmt *>(stmt)->inputPath;
    std::string fileName = fileRelPath.substr(fileRelPath.find_last_of("/") + 1);
    std::string fileDir = fileRelPath.substr(0, fileRelPath.find_last_of("/"));
    file << ".section .debug_line_str,\"MS\",@progbits,1\n"
            ".Ldebug_file_string: .string \"" << fileName << "\"\n"
            ".Ldebug_file_dir: .string \"" << fileDir << "\"\n";
  }
  file.close();

  output_filename =
      output_filename.substr(0, output_filename.find_last_of("."));

  ErrorClass::printError(); // Print any errors that may have occurred

  // Compile, but do not link main.o
  std::string assembler = // "Dont include standard libraries"
      (isDebug)
        ? "gcc -g -no-pie " + output_filename + ".s -o " + output_filename
        : "gcc -no-pie " + output_filename + ".s -o " + output_filename;
  std::string assembler_log = output_filename + "_assembler.log";
  if (!execute_command(assembler, assembler_log))
    return;
  
  int exitCode; // NOTE: This is to remove warnings. It is not practical lmao
  if (!isSaved) {
    std::string remove =
        "rm " + output_filename + ".s";
    exitCode = system(remove.c_str());
  }

  // delete the log files
  // (they are only necessary when something actually goes wrong,
  // and this code is only reached when we know it doesn't)
  std::string remove_log = "rm " + assembler_log;
  exitCode = system(remove_log.c_str());
}