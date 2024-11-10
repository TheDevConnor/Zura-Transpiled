#include "../helper/error/error.hpp"
#include "../common.hpp"

#include "gen.hpp"
#include "optimizer/optimize.hpp"
#include "optimizer/stringify.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

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

  // Get full realpath of the input file
  std::filesystem::path input_path = std::filesystem::absolute(filename);
  // absolute path of directory
  std::string input_dir = input_path.parent_path().string();
  std::string input_file = input_path.filename().string();
  // stmt->debug();
  visitStmt(stmt);

  // Make 2 passes of optimization
  text_section = Optimizer::optimizeInstrs(text_section);
  text_section = Optimizer::optimizeInstrs(text_section);
  
  // functions are pushed to text anyway so idk wtf im doing with my life anymore lol
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
    // Get name of filename - do not include its directory
    file << ".file \"" << input_file << "\"\n";
  file << ".text\n";
  file << ".globl main\n";
  if (debug) {
    for (size_t i = 0; i < fileIDs.size(); i++) {
      // Get the absolute path of the fileID path, which is relative to the path of the input file
      std::filesystem::path fileID_path = std::filesystem::absolute(std::filesystem::path(fileIDs.at(i)));
      std::string fileID_path_dir = fileID_path.parent_path().string();
      std::string fileID_path_file = fileID_path.filename().string();
      file << ".file " << i << " \"" + fileID_path_dir + "\" \"" + fileID_path_file + "\"\n";
    }
    file << ".Ltext0:\n.weak .Ltext0\n";
  }
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
  if (debug) 
    file << ".Ldebug_text0:\n";
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
	  file << ".section	.debug_info,\"\",@progbits\n\t";
    file << "\n.long .Ldebug_end - .Ldebug_info" // Length of the debug info header 
            "\n.Ldebug_info:" // NOTE: ^^^^ This long tag right here requires the EXCLUSION of those 4 bytes there
            "\n.weak .Ldebug_info"
            "\n.word 0x5" // DWARF version 5
            "\n.byte 0x1" // Unit-type DW_UT_compile
            "\n.byte 0x8" // 8-bytes registers (64-bit os)
            "\n.long .Ldebug_abbrev"
            "\n"
            "\n.uleb128 0x1" // Compilation unit name
            "\n.long .Ldebug_producer_string" // Producer - command or program that created this stinky assembly code
            "\n.byte 0x8042" // Custom Language (ZU) - not standardized in DWARF so we're allowed ot use it for "custom" purposes
            "\n.long .Ldebug_file_string" // Filename
            "\n.long .Ldebug_file_dir" // Filepath
            "\n.quad .Ltext0" // Low PC (beginning of .text)
            "\n.quad .Ldebug_text0 - .Ltext0" // High PC (end of .text)
            "\n.long .Ldebug_line0"
            "\n";
    // Attributes or whatever that follow
    file << Stringifier::stringifyInstrs(die_section) << "\n";
    file << ".Lint_debug_type:\n"
            ".uleb128 4\n"
            ".byte 8\n" // 8 bytes
            ".byte 5\n" // DW_ATE_signed - basically signed int
            ".string \"int\"\n";
    file << ".Lfloat_debug_type:\n"
            ".uleb128 4\n"
            ".byte 4\n"
            ".byte 4\n"
            ".string \"float\"\n";
    file << ".Lstr_debug_type:\n"
            ".uleb128 5\n" // Index label
            ".byte 8\n" // 8 bytes (pointers are 8 bytes in x64)
            ".long .Lchar_debug_type\n"; // char*
    file << ".Lchar_debug_type:\n"
            ".uleb128 4\n" // Index label
            ".byte 1\n" // 1 byte
            ".byte 8\n" // DW_ATE_signed
            ".string \"char\"\n";
    file << ".byte 0\n"; // End of the children of the compilation unit
    file << ".Ldebug_end:\n";
    file << ".section .debug_abbrev,\"\",@progbits\n";
    file << ".Ldebug_abbrev:\n";
    file << "\n.uleb128 0x1" // Index label
            "\n.uleb128 0x11" // DW_TAG_compile_unit
            "\n.byte	0x1" // Has children
            "\n.uleb128 0x25" // DW_AT_producer
            "\n.uleb128 0xe" // DW_FORM_strp
            "\n.uleb128 0x13" // DW_AT_language
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x3" // DW_AT_name
            "\n.uleb128 0x1f" // DW_FORM_line_strp
            "\n.uleb128 0x1b" // DW_AT_comp_dir
            "\n.uleb128 0x1f" // DW_FORM_line_strp
            "\n.uleb128 0x11" // DW_AT_low_pc
            "\n.uleb128 0x1" // DW_FORM_addr
            "\n.uleb128 0x12" // DW_AT_high_pc
            "\n.uleb128 0x7" // DW_FORM_data8
            "\n.uleb128 0x10" // DW_AT_stmt_list
            "\n.uleb128 0x17" // DW_FORM_sec_offset
            "\n.byte	0" // End of attributes list
            "\n.byte	0"
            "\n.uleb128 0x2" // Index label
            "\n.uleb128 0x2e" // DW_TAG_subprogram
            "\n.byte	0x1" // Has children
            "\n.uleb128 0x3f" // DW_AT_external
            "\n.uleb128 0x19" // DW_FORM_flag_present
            "\n.uleb128 0x3" // DW_AT_name
            "\n.uleb128 0xe" // DW_FORM_strp
            "\n.uleb128 0x3a" // DW_AT_decl_file
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x3b" // DW_AT_decl_line
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x39" // DW_AT_decl_column
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x49" // DW_AT_type
            "\n.uleb128 0x13" // DW_FORM_ref4
            "\n.uleb128 0x11" // DW_AT_low_pc
            "\n.uleb128 0x1" // DW_FORM_addr
            "\n.uleb128 0x12" // DW_AT_high_pc
            "\n.uleb128 0x7" // DW_FORM_data8
            "\n.uleb128 0x40" // DW_AT_frame_base
            "\n.uleb128 0x18" // DW_FORM_exprloc
            "\n.uleb128 0x7a" // DW_AT_call_all_calls
            "\n.uleb128 0x19" // DW_FORM_flag_present
            "\n.byte	0" // End of attributes list
            "\n.byte	0"
            "\n.uleb128 0x3" // Index label
            "\n.uleb128 0x34" // DW_TAG_variable
            "\n.byte	0" // No children
            "\n.uleb128 0x3" // DW_AT_name
            "\n.uleb128 0xe" // DW_FORM_strp
            "\n.uleb128 0x3a" // DW_AT_decl_file
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x3b" // DW_AT_decl_line
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x39" // DW_AT_decl_column
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x49" // DW_AT_type
            "\n.uleb128 0x13" // DW_FORM_ref4
            "\n.uleb128 0x2" // DW_AT_location
            "\n.uleb128 0x18" // DW_FORM_exprloc
            "\n.byte	0" // End of attributes list
            "\n.byte	0"
            "\n.uleb128 0x4" // Index label
            "\n.uleb128 0x24" // DW_TAG_base_type
            "\n.byte	0" // No children
            "\n.uleb128 0xb" // DW_AT_byte_size
            "\n.uleb128 0xb" // DW_FORM_data1 (1 byte)
            "\n.uleb128 0x3e" // DW_AT_encoding
            "\n.uleb128 0xb" // DW_FORM_data1 (1 byte)
            "\n.uleb128 0x3" // DW_AT_name
            "\n.uleb128 0x8" // DW_FORM_string
            "\n.byte	0" // End of attributes list
            "\n.byte	0"
            "\n.uleb128 0x5" // Index label
            "\n.uleb128 0xF" // DW_TAG_pointer_type
            "\n.byte 0" // No children
            "\n.uleb128 0xB" // DW_AT_byte_size
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x49" // DW_AT_type
            "\n.uleb128 0x13" // DW_FORM_ref4
            "\n.byte 0"
            "\n.byte 0"
             // Void function (no 'type' attribute)
            "\n.uleb128 0x6" // Index label
            "\n.uleb128 0x2e" // DW_TAG_subprogram
            "\n.byte	0x1" // Has children
            "\n.uleb128 0x3f" // DW_AT_external
            "\n.uleb128 0x19" // DW_FORM_flag_present
            "\n.uleb128 0x3" // DW_AT_name
            "\n.uleb128 0xe" // DW_FORM_strp
            "\n.uleb128 0x3a" // DW_AT_decl_file
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x3b" // DW_AT_decl_line
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x39" // DW_AT_decl_column
            "\n.uleb128 0xb" // DW_FORM_data1
            "\n.uleb128 0x11" // DW_AT_low_pc
            "\n.uleb128 0x1" // DW_FORM_addr
            "\n.uleb128 0x12" // DW_AT_high_pc
            "\n.uleb128 0x7" // DW_FORM_data8
            "\n.uleb128 0x40" // DW_AT_frame_base
            "\n.uleb128 0x18" // DW_FORM_exprloc
            "\n.uleb128 0x7a" // DW_AT_call_all_calls
            "\n.uleb128 0x19" // DW_FORM_flag_present
            // These bytes signal the end of the DIE abbreviations, by making
            // a null entry with NULL tag, no children, and no attributes.
            "\n.byte 0" // End of all children
            "\n.byte 0" // NULL Tag
            "\n.byte 0" // No more children
            "\n.byte 0\n"; // No more attributes
    
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
            ".Ldebug_producer_string: .string \"Zura compiler " + ZuraVersion + "\"\n";
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
        ? "gcc -g -e main -nostdlib -no-pie " + output_filename + ".s -o " + output_filename
        : "gcc -e main -nostdlib -no-pie " + output_filename + ".s -o " + output_filename;
  std::string assembler_log = output_filename + "_assembler.log";
  // loop over linkedFiles set and link them with gcc
  for (auto &linkedFile : linkedFiles) {
    assembler += " -l" + linkedFile;
  }
  if (!execute_command(assembler, assembler_log))
    return;
  
  int exitCode; // NOTE: This is to remove warnings. It is not practical lmao
  if (!isSaved) {
    std::string remove =
        "rm " + output_filename + ".s";
    exitCode = system(remove.c_str());
    if (exitCode) {
      // If its not zero, then it errored!
      // But I don't care. This command specifically does not matter.
    }
  }

  // delete the log files
  // (they are only necessary when something actually goes wrong,
  // and this code is only reached when we know it doesn't)
  std::string remove_log = "rm " + assembler_log;
  exitCode = system(remove_log.c_str());
}