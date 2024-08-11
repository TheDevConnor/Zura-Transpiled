#include "gen.hpp"

#include <iostream>
#include <string>
#include <fstream> // Add this line

void codegen::push(std::string str) {
    output_code += str;

    if (str.back() != '\n') {
        output_code += '\n';
    }
}

void codegen::gen(Node::Stmt* stmt, bool isSaved, std::string output) {
    // Exmaple Asm code using the push function with nasam syntax
    // push("section .text");
    push("global _start");
    push("_start:");
    push("mov eax, 1");
    push("mov ebx, 2");
    push("add eax, ebx");
    push("mov ebx, 0");
    push("int 0x80");

    if (isSaved) {
        std::ofstream file(output + ".asm");
        file << output_code;
        file.close();
    } else {
        std::cout << output_code;
    }

    output_code.clear();
}