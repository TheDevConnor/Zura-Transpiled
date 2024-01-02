#include <fstream>
#include <cstring>

#include "../ast.hpp"

void AstNode::codeGen(AstNode *expression) {
    std::ofstream file;
    file.open("out.s");
    
    file << ".intel_syntax noprefix\n";
    file << ".global main\n";
    file << ".extern exit\n";
    
    if (expression->type == AstNodeType::PROGRAM) {
        AstNode::Program *program = (AstNode::Program *)expression->data;
    
        for (AstNode *stmt : program->statements) {
        switch (stmt->type) {
        case AstNodeType::FUNCTION_DECLARATION: {
            AstNode::FunctionDeclaration *functionDeclaration =
                (AstNode::FunctionDeclaration *)stmt->data;
    
            file << functionDeclaration->name.start << ":\n";
    
            file << "  push rbp\n";
            file << "  mov rbp, rsp\n";
            file << "  sub rsp, 16\n";
    
            file << "  mov rax, rdi\n";
            file << "  mov rdi, rax\n";
            file << "  mov rax, 0\n";
            file << "  call exit\n";
    
            file << "  mov rsp, rbp\n";
            file << "  pop rbp\n";
            file << "  ret\n";
            break;
        }
        default:
            break;
        }
        }
    }
    
    file.close();
}