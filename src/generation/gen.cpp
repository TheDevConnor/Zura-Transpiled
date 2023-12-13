#include <iostream>
#include <fstream>
#include <sstream>

#include "gen.hpp"

Gen::Gen(AstNode* ast) : ast(ast) {
    generate();
}

void Gen::generate() {
    std::cout << "Creating file..." << std::endl;
    std::ofstream file;
    file.open("out.c");

    std::cout << "Writing header..." << std::endl;
    file << "// -----------------------------------------\n";
    file << "// Compile Zura to C\n";
    file << "// -----------------------------------------\n\n";

    file << "#include <stdio.h>\n\n";

    std::cout << "Generating the code" << std::endl;

    ast->printAst(ast, 0);

    std::cout << "Done!" << std::endl;
}