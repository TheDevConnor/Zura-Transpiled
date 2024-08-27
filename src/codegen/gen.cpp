#include "../common.hpp"

#include "gen.hpp"
#include "optimize.hpp"
#include "stringify.hpp"
#include "../common.hpp"
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

	std::ofstream file(output_filename + ".s");
	if (file.is_open()) {
		file << "# ┌-------------------------------┐\n"
				"# |   Zura lang by TheDevConnor   |\n"
				"# |  assembly by Soviet Pancakes  |\n"
				"# └-------------------------------┘\n"
				"# " << ZuraVersion << "\n"
				"# What's New: Arrays!\n"
				"# Help Needed! Bug with 'arr[0]' and 'arr[1]' where 0 and 1 are returned instead of array elements\n";
		// Copying gcc and hoping something changes (it won't)
		// .file directive does not like non-c and non-cpp files but it might be useful for something somewhere later
		file << "#.file \"" << static_cast<ProgramStmt *>(stmt)->inputPath << "\"\n" <<
				".text\n" 
				".globl main\n" 
				".type main, @function\n" 
				"main:\n" 
				"  .cfi_startproc\n" 
				"  pushq %rbp\n" 
				"  movq %rsp, %rbp\n" 
				"  call usr_main\n"
				"  mov %rax, %rdi\n" 
				"  movq $60, %rax\n" 
				"  syscall\n" 
				"  movq %rbp, %rsp\n" 
				"  pop %rbp\n" 
				"  ret\n" 
				"  .cfi_endproc\n" 
				".size main, .-main\n";
		file << Stringifier::stringifyInstrs(text_section); 
		file << "\n# zura functions\n";
		if (nativeFunctionsUsed[NativeASMFunc::strlen] == true) {
			file << "\nnative_strlen:"
							"\n  push  %rbx             ; save any registers that "
							"\n  push  %rcx             ; we will trash in here"
							"\n  mov   %rdi, %rbx       ; rbx = rdi"
							"\n  xor   %al, %al         ; look for NUL term (0x0)"
							"\n  mov   %rcx, 0xffffffff ; the max string length is 4gb"
							"\n  repne scasb            ; while [rdi] != al && rcx > 0, rdi++"
							"\n  sub   %rdi, %rbx       ; length = end - start"
							"\n  mov   %rdi, %rax       ; rax now holds our length"
							"\n  pop   %rcx             ; restore the saved registers"
							"\n  pop   %rbx"
							"\n  ret\n";
		}
		file << "\n# non-main user functions" << std::endl;
		file << Stringifier::stringifyInstrs(head_section);
		file << "\n# data section for string and stuff"
						"\n.data\n";
		file << Stringifier::stringifyInstrs(data_section);
		file.close();
	} else {
		std::cerr << "Unable to open output file for finalized assembly '" << output_filename << ".s' - ensure Zura has permissions to write/create files" << std::endl;
	}

	output_filename = output_filename.substr(0, output_filename.find_last_of("."));

	// Compile, but do not link main.o
	std::string assembler = "gcc -c " + output_filename + ".s -o " + output_filename + ".o";
	const char *assembler_cstr = assembler.c_str();
	system(assembler_cstr);

	// Link with entry point "main"
	std::string linker = "ld " + output_filename + ".o -e main -o " + output_filename; 
	const char *linker_cstr = linker.c_str(); // is it an issue with the asm?
	system(linker_cstr);

	if (!isSaved) {
		std::string remove = "rm " + output_filename + ".s " + output_filename + ".o";
		system(remove.c_str());
	}
}