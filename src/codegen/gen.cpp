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
  if (!file.is_open()) handleError(0, 0,
              "Unable to open output file for finalized assembly '" +
                  output_filename +
                  ".s' - ensure Zura has permissions to write/create files", "Codegen Error");
  
  file << "# ┌---------------------------------┐\n"
          "# |   Zura Syntax by TheDevConnor   |\n"
          "# |   assembly by Soviet Pancakes   |\n"
          "# └---------------------------------┘\n"
          "# "
        << ZuraVersion
        << "\n"
          "# What's New: Struct Arrays, among bug fixes\n"
        << "\n# Everything beyond this point was generated automatically by the Zura compiler.\n" 
           ".att_syntax\n";
  

  // This one defines the file the whole assembly is related to
  if (debug)
    // Get name of filename - do not include its directory
    file << ".file \"" << input_file << "\"\n";
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
  file << "_start:\n"
          "  call main\n"
          "  xorq %rdi, %rdi\n"
          "  movq $60, %rax\n"
          "  syscall\n";
  file << Stringifier::stringifyInstrs(text_section, debug);
  if (nativeFunctionsUsed.size() > 0) file << "\n# zura functions\n";
  if (nativeFunctionsUsed[NativeASMFunc::strlen] == true) {
    file << ".type native_strlen, @function"
            "\nnative_strlen:"
            "\n  #input in rdi"
            "\n  movb $0, %al"
            "\n  movq $0xFFFFFFF, %rcx"
            "\n  repne scasb"
            "\n  movq $0xFFFFFFF, %rbx"
            "\n  subq %rcx, %rbx"
            "\n  movq %rbx, %rax"
            "\n  ret"
            "\n.size native_strlen, .-native_strlen\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::itoa] == true) {
    file << ".type native_itoa, @function\n"
          "native_itoa:" // Unsigned itoa... Does not check for '-' sign and will print 0xFFFFFFFFFFFFFFFF (18446744073709551615) as -1 so don't do that...
          "\n    # Allocate space for the string on the stack"
          "\n    subq $32, %rsp          # Allocate 32 bytes for the buffer"
          "\n    movq %rsp, %rcx         # Save buffer address in %rcx"
          "\n    addq $31, %rcx          # Point to the end of the buffer (minus null terminator)"
          "\n    movb $0, (%rcx)         # Null-terminate the string"
          "\n    # Check if the input is zero"
          "\n    movq %rdi, %rax         # Copy input number to %rax"
          "\n    testq %rax, %rax        # Check if %rax == 0"
          "\n    jnz .uitoa_loop         # If not zero, process the digits"
          "\n    # Special case for zero"
          "\n    movb $'0', -1(%rcx)     # Store '0' before the null terminator"
          "\n    leaq -1(%rcx), %rax     # Return pointer to the string in %rax"
          "\n    addq $32, %rsp          # Restore the stack"
          "\n    ret"
          "\n.uitoa_loop:"
          "\n    # Process each digit"
          "\n    movq $10, %rbx          # Divisor = 10"
          "\n.uitoa_div:"
          "\n    xorq %rdx, %rdx         # Clear %rdx for division"
          "\n    divq %rbx               # Divide %rax by 10, remainder in %rdx, quotient in %rax"
          "\n    addb $'0', %dl          # Convert remainder to ASCII"
          "\n    decq %rcx               # Move buffer pointer backward BEFORE storing character"
          "\n    movb %dl, (%rcx)        # Store ASCII character at the current buffer position"
          "\n    testq %rax, %rax        # Check if quotient is zero"
          "\n    jnz .uitoa_div          # Repeat if quotient != 0"
          "\n    # Prepare return value"
          "\n    leaq (%rcx), %rax       # Return pointer to the start of the string in %rax"
          "\n    addq $32, %rsp          # Restore the stack"
          "\n    ret"
          "\n"
          ".size native_itoa, .-native_itoa\n";
  }
  if (nativeFunctionsUsed[NativeASMFunc::ftoa] == true) {
    file << ".type native_ftoa, @function\n"
            "native_ftoa:\n"
            "    subq $64, %rsp          # Allocate 64 bytes for the buffer\n"
            "    movq %rsp, %rcx         # Save buffer address in %rcx\n"
            "    addq $63, %rcx          # Point to the end of the buffer\n"
            "    movb $0, (%rcx)         # Null-terminate the string\n"

            "    # Handle the integer part\n"
            "    cvttsd2si %xmm0, %rax   # Convert float to integer\n"
            "    testq %rax, %rax        # Check if integer part is zero\n"
            "    jnz .ftoa_int_loop      # Process digits if non-zero\n"

            "    # Special case for zero\n"
            "    movb $'0', -1(%rcx)     # Store '0'\n"
            "    leaq -1(%rcx), %rax     # Return pointer\n"
            "    addq $64, %rsp          # Restore stack\n"
            "    ret\n"

            ".ftoa_int_loop:\n"
            "    movq $10, %rbx          # Divisor\n"
            ".ftoa_int_div:\n"
            "    xorq %rdx, %rdx         # Clear %rdx for division\n"
            "    divq %rbx               # Divide %rax by 10\n"
            "    addb $'0', %dl          # Convert remainder to ASCII\n"
            "    decq %rcx               # Move buffer pointer\n"
            "    movb %dl, (%rcx)        # Store ASCII digit\n"
            "    testq %rax, %rax        # Check if quotient is zero\n"
            "    jnz .ftoa_int_div       # Repeat if non-zero\n"

            "    # Add decimal point\n"
            "    movb $'.', -1(%rcx)\n"
            "    decq %rcx\n"

            "    # Handle fractional part\n"
            "    movsd %xmm0, %xmm1      # Copy original float\n"
            "    roundsd $0b00000000, %xmm1, %xmm1 # Truncate\n"
            "    subsd %xmm1, %xmm0      # Subtract integer part\n"
            "    movq $10, %rax          # Multiplier\n"
            "    movq $6, %rsi           # Precision = 6 decimal places\n"

            ".ftoa_frac_loop:\n"
            "    mulsd %xmm0, %xmm0      # Multiply fractional part by 10\n"
            "    cvttsd2si %xmm0, %rdi   # Convert to integer\n"
            "    addb $'0', %dil         # Convert to ASCII\n"
            "    movb %dil, -1(%rcx)     # Store digit\n"
            "    decq %rcx\n"
            "    movsd %xmm0, %xmm1      # Truncate fractional part\n"
            "    roundsd $0b00000000, %xmm1, %xmm1\n"
            "    subsd %xmm1, %xmm0      # Subtract integer part\n"
            "    decq %rsi               # Decrease precision\n"
            "    jnz .ftoa_frac_loop     # Repeat\n"

            "    # Return result\n"
            "    leaq (%rcx), %rax\n"
            "    addq $64, %rsp\n"
            "    ret\n"
            "\n"
            ".size native_ftoa, .-native_ftoa\n";
  }
  if (debug) 
    file << ".Ldebug_text0:\n";
  if (head_section.size() > 0) {
    file << "\n# non-main user functions" << std::endl;
    file << Stringifier::stringifyInstrs(head_section, debug);
  }
  if (rodt_section.size() > 0) {
    file << "\n# readonly data section - contains constant strings and floats (for now)"
            "\n.section .rodata\n";
    file << Stringifier::stringifyInstrs(rodt_section, debug);
  }
  if (data_section.size() > 0) {
    file << "\n# data section for pre-allocated, mutable data"
            "\n.data\n";
    file << Stringifier::stringifyInstrs(data_section, debug);
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
    file << Stringifier::stringifyInstrs(die_section, debug) << "\n";
    file << Stringifier::stringifyInstrs(diet_section, debug) << "\n";
    // Ensure that these TYPES are public
    // If they are INSIDE the compile unit (before the byte 0 above here),
    // then they are not visible to other CU's (other files)
    if (dwarf::isUsed(dwarf::DIEAbbrev::Type)) {
      file << ".Lint_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Type) +
            "\n.byte 8\n" // 8 bytes
            ".byte 5\n" // DW_ATE_signed - basically `signed long long int` in c
            ".string \"int\"\n";
      file << ".Llong_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Type) +
            "\n.byte 4\n" // 4 bytes
            ".byte 7\n" // DW_ATE_unsigned - basically `unsigned long int` in c
            ".string \"long\"\n";
      file << ".Lbool_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Type) +
            "\n.byte 1\n" // 1 byte
            ".byte 0x02\n" // DW_ATE_boolean - basically `bool` in c
            ".string \"bool\"\n";
    file << ".Lfloat_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Type) +
            "\n.byte 4" // 4 bytes
            "\n.byte 4" // DW_ATE_float - basically `float` in c, or a "single precision, scalar" float in 🤓 CPU terms
            "\n.string \"float\"\n";
    // Str type is a pointer type
    // Might as well just let it in.
    dwarf::useAbbrev(dwarf::DIEAbbrev::PointerType);
    file << ".Lstr_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::PointerType) +
            "\n.byte 8\n" // 8 bytes (pointers are 8 bytes in x64)
            ".long .Lchar_debug_type\n"; // char*
    file << ".Lchar_debug_type:\n"
            ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Type) +
            "\n.byte 1\n" // 1 byte
            ".byte 6\n" // DW_ATE_signed
            ".string \"char\"\n";
    }
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
    file << Stringifier::stringifyInstrs(dies_section, debug) << "\n";
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

  output_filename =
      output_filename.substr(0, output_filename.find_last_of("."));

  ErrorClass::printError(); // Print any errors that may have occurred

  // Compile, but do not link main.o
  std::string assembler = // "Dont include standard libraries"
      (isDebug)
        ? "gcc -g -e _start -nostdlib -nostartfiles -no-pie " + output_filename + ".s -o " + output_filename
        : "gcc -e _start -nostdlib -nostartfiles -no-pie " + output_filename + ".s -o " + output_filename;
  std::string assembler_log = output_filename + "_assembler.log";
  // loop over linkedFiles set and link them with gcc
  for (std::string linkedFile : linkedFiles) {
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