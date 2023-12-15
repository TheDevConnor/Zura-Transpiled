#include <fstream>
#include <cstring>

#include "../ast/ast.hpp"
#include "gen.hpp"

void Gen::functionDeclaration(std::ofstream &file, AstNode::FunctionDeclaration *functionDeclaration) {
    // Find the type
    const char* type = findType(functionDeclaration->type);
    printTypeToFile(file, type);

    // Find the name
    functionDeclaration->name.start = strtok(const_cast<char *>(functionDeclaration->name.start), "(");
    file << functionDeclaration->name.start << "(";

    // TODO: Implement parameters to generation
    file << ")";
    
    // Find the body
    file << " {\n";

    // Find the statements
    if (functionDeclaration->body->type == AstNodeType::BLOCK) {
        AstNode::Block *block = (AstNode::Block *)functionDeclaration->body->data;
        for (AstNode *stmt : block->statements) {
            if (stmt->type == AstNodeType::EXIT) {
                AstNode::Exit *exit = (AstNode::Exit *)stmt->data;
                if (exit->expression->type == AstNodeType::NUMBER_LITERAL) {
                AstNode::NumberLiteral *number = (AstNode::NumberLiteral *)exit->expression->data;
                file << "  return " << number->value << ";\n";
                }
            }
        }
    }
    
    file << "}\n\n";
}