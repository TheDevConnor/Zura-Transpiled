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

  // Make 3 passes of optimization
  text_section = Optimizer::optimizeInstrs(text_section);
  text_section = Optimizer::optimizeInstrs(text_section);
  text_section = Optimizer::optimizeInstrs(text_section);
  
  // data section cannot be optimized
  // rodata section cant be optimized either

  std::ofstream file(output_filename + ".s");
  if (!file.is_open()) handleError(0, 0,
              "Unable to open output file for finalized assembly '" +
                  output_filename +
                  ".s' - ensure Zura has permissions to write/create files", "Codegen Error");
  
  file << "# ╔═════════════════════════════════╗\n"
          "# ║   Zura Syntax by TheDevConnor   ║\n"
          "# ║   assembly by Soviet Pancakes   ║\n"
          "# ╚═════════════════════════════════╝\n"
          "# "
        << ZuraVersion
        << "\n"
          "# What's New: Dereferencing structs\n"
        << "\n# Everything beyond this point was generated automatically by the Zura compiler.\n" 
           ".att_syntax\n";
  

  // This one defines the file the whole assembly is related to
  if (debug)
    // Get name of filename - do not include its directory
    file << ".file \"" << input_file << "\"\n";
  // Before the text, we will output the BSS section
  if (useArguments) {
    file << ".bss\n"
            ".Largc:\n"
            "  .type .Largc, @object\n"
            "  .zero 8\n"
            "  .size .Largc, 8\n"
            "\n"
            ".Largv:\n"
            "  .type .Largv, @object\n"
            "  .zero 8\n"
            "  .size .Largv, 8\n"
            "\n";
  }
  file << ".text\n";
  file << ".globl _start\n";
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
  if (useArguments) {
    file << "_start:\n"
            "  .cfi_startproc\n"
            "  movq (%rsp), %rax\n"
            "  movq %rax, .Largc(%rip)\n"
            "  leaq 8(%rsp), %rax\n"
            "  movq %rax, .Largv(%rip)\n"
            "  call main\n"
            "  xorq %rdi, %rdi\n"
            "  movq $60, %rax\n"
            "  syscall\n"
            "  .cfi_endproc\n"
            ".size _start, .-_start\n";
  } else {
    file << "_start:\n"
            "  .cfi_startproc\n"
            "  call main\n"
            "  xorq %rdi, %rdi\n"
            "  movq $60, %rax\n"
            "  syscall\n"
            "  .cfi_endproc\n"
            ".size _start, .-_start\n";
  }
  file << Stringifier::stringifyInstrs(text_section);
  if (nativeFunctionsUsed.size() > 0) file << "\n# zura functions\n";
  if (nativeFunctionsUsed[NativeASMFunc::strlen] == true) {
    file << ".type native_strlen, @function"
            "\nnative_strlen:"
            "\n  #input in rdi"
            "\n  movb $0, %al"
            "\n  movq $0xFFFFFFF, %rcx"
            "\n  repne scasb # repeat while (%rdi) not zero, decrease %rcx"
            "\n  movq $0xFFFFFFF, %rbx"
            "\n  subq %rcx, %rbx # subtract to get total length"
            "\n  movq %rbx, %rax"
            "\n  decq %rax # the rep instruction also includes the NUL, so we must remove it here"
            "\n  ret"
            "\n.size native_strlen, .-native_strlen\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::uitoa] == true) {
    file << ".type native_uitoa, @function\n"
          "native_uitoa:"
          "\n    # Allocate space for the string on the stack"
          "\n    subq $32, %rsp          # Allocate 32 bytes for the buffer"
          "\n    movq %rsp, %rcx         # Save buffer address in %rcx"
          "\n    addq $31, %rcx          # Point to the end of the buffer (minus null terminator)"
          "\n    movb $0, (%rcx)         # Null-terminate the string"
          "\n    # Check if the input is zero"
          "\n    movq %rdi, %rax         # Copy input number to %rax"
          "\n    testq %rax, %rax        # Check if %rax == 0"
          "\n    jnz .uitoa_loop    # If not zero, process the digits"
          "\n    # Special case for zero"
          "\n    movb $'0', -1(%rcx)     # Store '0' before the null terminator"
          "\n    leaq -1(%rcx), %rax     # Return pointer to the string in %rax"
          "\n    addq $32, %rsp          # Restore the stack"
          "\n    ret"
          "\n.uitoa_loop:"
          "\n    # Process each digit"
          "\n    movq $10, %rbx          # Divide by 10"
          "\n.uitoa_div:"
          "\n    xorq %rdx, %rdx         # Clear %rdx for division"
          "\n    divq %rbx               # Divide %rax by 10"
          "\n    addb $'0', %dl          # Convert remainder to ASCII"
          "\n    decq %rcx               # Move buffer pointer"
          "\n    movb %dl, (%rcx)        # Store ASCII digit"
          "\n    testq %rax, %rax        # Check if quotient is zero"
          "\n    jnz .uitoa_div          # Repeat if quotient != 0"
          "\n    # Prepare return value"
          "\n    leaq (%rcx), %rax       # Return pointer to the start of the string in %rax"
          "\n    addq $32, %rsp          # Restore the stack"
          "\n    ret"
          "\n"
          ".size native_uitoa, .-native_uitoa\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::itoa] == true) {
    file << ".type native_itoa, @function\n"
            "native_itoa:"
            "\n    # Allocate space for the string on the stack"
            "\n    subq $32, %rsp          # Allocate 32 bytes (characters) for the buffer"
            "\n    movb $0, %r8b           # used to store the - sign if necessary"
            "\n    movq %rsp, %rcx         # Save buffer address in %rcx"
            "\n    addq $31, %rcx          # Point to the end of the buffer (minus null terminator)"
            "\n    movb $0, (%rcx)         # Null-terminate the string"
            "\n    movq %rdi, %rax         # Copy input number to %rax"
            "\n    testq %rax, %rax           # Test if input is already zero (we cannot do the loop if so)"
            "\n    jns .itoa_zero_check"
            "\n    movb $'-', %r8b"
            "\n    negq %rax"
            "\n.itoa_zero_check:"
            "\n    jnz .itoa_loop          # If not zero, process the digits"
            "\n    # Special case for zero"
            "\n    movb $'0', -1(%rcx)     # Store '0' before the null terminator"
            "\n    leaq -1(%rcx), %rax     # Return pointer to the string in %rax"
            "\n    addq $32, %rsp          # Restore the stack"
            "\n    ret"
            "\n.itoa_loop:"
            "\n    # Process each digit"
            "\n    movq $10, %rbx          # Divisor = 10"
            "\n.itoa_div:"
            "\n    xorq %rdx, %rdx         # Clear %rdx for division"
            "\n    divq %rbx               # Divide %rax by 10, remainder in %rdx, quotient in %rax"
            "\n    addb $'0', %dl          # Convert remainder to ASCII"
            "\n    decq %rcx               # Move buffer pointer backward BEFORE storing character"
            "\n    movb %dl, (%rcx)        # Store ASCII character at the current buffer position"
            "\n    testq %rax, %rax        # Check if quotient is zero"
            "\n    jnz .itoa_div           # Repeat if quotient != 0"
            "\n    # Prepare return value"
            "\n    decq %rcx"
            "\n    movb %r8b, (%rcx)"
            "\n    leaq (%rcx), %rax       # Return pointer to the start of the string in %rax"
            "\n    addq $32, %rsp          # Restore the stack"
            "\n    ret"
            "\n"
            ".size native_itoa, .-native_itoa\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::memcpy] == true) {
    file << ".type native_memcpy, @function\n"
          "native_memcpy:"
          "\n    # rdi = dest, rsi = src, rdx = bytes"
          "\n    movq %rdx, %rcx"
          "\n    rep movsb"
          "\n    ret"
          "\n.size native_memcpy, .-native_memcpy\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::strcmp] == true) {
    file << ".type native_strcmp, @function\n"
            "native_strcmp:"
            "\n    # rdi = str1, rsi = str2"
            "\n    xorq %rax, %rax"
            "\n    movq $0, %rbx # count"
            "\n.loop:"
            "\n    movb (%rdi, %rbx), %cl"
            "\n    movb (%rsi, %rbx), %dl"
            "\n    cmpb %cl, %dl"
            "\n    jne .notequal"
            // null terminator check
            "\n    cmpb $0, %cl"
            "\n    je .equal"
            "\n    incq %rbx"
            "\n    jmp .loop"
            "\n.equal:"
            "\n    movq $1, %rax"
            "\n    ret"
            "\n.notequal:"
            "\n    movq $0, %rax"
            "\n    ret"
            "\n.size native_strcmp, .-native_strcmp\n";
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
            "\n.uleb128 " + std::to_string((int)dwarf::DIEAbbrev::CompileUnit) + // Compilation unit name
            "\n.long .Ldebug_producer_string - .Ldebug_str_start" // Producer - command or program that created this stinky assembly code
            "\n.short 0x8042" // Custom Language (ZU) - not standardized in DWARF so we're allowed ot use it for "custom" purposes
            "\n.long .Ldebug_file_string - .Ldebug_line_str_start" // Filename
            "\n.long .Ldebug_file_dir - .Ldebug_line_str_start" // Filepath
            "\n.quad .Ltext0" // Low PC (beginning of .text)
            "\n.quad .Ldebug_text0 - .Ltext0" // High PC (end of .text)
            "\n.long .Ldebug_line0"
            "\n";
    // Attributes or whatever that follow
    file << Stringifier::stringifyInstrs(die_section) << "\n";
    dwarf::emitTypes();
    file << Stringifier::stringifyInstrs(diet_section) << "\n"; // types, both builtin and user-defined
    // If they are INSIDE the compile unit (before the byte 0 above here),
    // then they are not visible to other CU's (other files)
    file << ".byte 0\n"; // End of compile unit's children -- THIS ACTUALLY NEEDS TO GO HERE!
    file << ".Ldebug_end:\n";
    file << ".section .debug_abbrev,\"\",@progbits\n";
    file << ".Ldebug_abbrev:\n";
    dwarf::useAbbrev(dwarf::DIEAbbrev::CompileUnit); // required
    file << dwarf::generateAbbreviations();
    
    // I dont think the debug_aranges section is totally required
    // so im not wasting brainpower on it

    // debug_line
    file << ".section .debug_line,\"\",@progbits\n";
    file << ".Ldebug_line0:\n";
    // TODO
    // debug_str
    file << ".section .debug_str,\"MS\",@progbits,1\n"
            ".Ldebug_str_start:\n"
            ".Ldebug_producer_string: .string \"Zura compiler " + ZuraVersion + "\"\n";
    file << Stringifier::stringifyInstrs(dies_section) << "\n";
    // debug_line_str
    std::string fileRelPath = static_cast<ProgramStmt *>(stmt)->inputPath;
    std::string fileName = fileRelPath.substr(fileRelPath.find_last_of("/") + 1);
    std::string fileDir = fileRelPath.substr(0, fileRelPath.find_last_of("/"));
    file << ".section .debug_line_str,\"MS\",@progbits,1\n"
            ".Ldebug_line_str_start:\n"
            ".Ldebug_file_string: .string \"" << fileName << "\"\n"
            ".Ldebug_file_dir: .string \"" << fileDir << "\"\n";
  }
  file.close();

  output_filename = output_filename.substr(0, output_filename.find_last_of("."));

  bool isError = Error::report_error();
  if (isError) { return; }

  // Compile, but do not link main.o
  std::string assembler = // "Dont include standard libraries"
      (isDebug)
        ? "gcc -g -e _start -nostdlib -nostartfiles " + output_filename + ".s -o " + output_filename
        : "gcc -e _start -nostdlib -nostartfiles " + output_filename + ".s -o " + output_filename;
  std::string assembler_log = output_filename + "_assembler.log";

  // loop over linkedFiles set and link them with gcc
  for (std::string linkedFile : linkedFiles) assembler += " -l" + linkedFile;

  bool success = execute_command(assembler, assembler_log);
  if (!success) return;
  
  int exitCode; // NOTE: This is to remove warnings. It is not practical lmao
  if (!isSaved) {
    std::string remove = "rm " + output_filename + ".s";
    exitCode = system(remove.c_str());
    if (exitCode) {
      std::string msg = "Error removing file '" + output_filename + ".s'";
      handleError(0, 0, msg, "Codegen Error");
      return;
    }
  }

  // delete the log files
  // (they are only necessary when something actually goes wrong,
  // and this code is only reached when we know it doesn't)
  std::string remove_log = "rm " + assembler_log;
  exitCode = system(remove_log.c_str());
}