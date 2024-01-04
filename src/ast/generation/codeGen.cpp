#include <fstream>
#include <iostream>
#include <cstring>
#include <unordered_map>

#include "../../lexer/lexer.hpp"
#include "../ast.hpp"
#include "codeGen.hpp"

const std::unordered_map<TokenKind, std::string> CodeGen::typeMapping = {
    {TokenKind::I8,     "db"},
    {TokenKind::U8,     "db"},
    {TokenKind::Bool,   "db"},
    {TokenKind::STRING, "db"},

    {TokenKind::I16, "dw"},
    {TokenKind::U16, "dw"},

    {TokenKind::I32, "dd"},
    {TokenKind::F32, "dd"},
    {TokenKind::U32, "dd"},

    {TokenKind::I64, "dq"},
    {TokenKind::F64, "dq"},
    {TokenKind::U64, "dq"}
};

void CodeGen::findType(AstNode *type, std::ofstream &file) {
    AstNode::Type *typeNode = static_cast<AstNode::Type *>(type->data);
    TokenKind kind = typeNode->type.kind;

    auto it = typeMapping.find(kind);
    if (it != typeMapping.end()) {
        file << it->second;
    } else {
        std::cout << "Unknown type\n";
    }
}

void CodeGen::generateVariableDeclaration(AstNode::VarDeclaration *variable, std::ofstream &file) {
    if (variable->initializer->type == AstNodeType::NUMBER_LITERAL) {
        AstNode::NumberLiteral *number = static_cast<AstNode::NumberLiteral *>(variable->initializer->data);
        variable->name.start = strtok(const_cast<char*>(variable->name.start), ":");
        file << "   " << variable->name.start << " ";
        findType(variable->type, file);
        file << " " << number->value << "\n";
    }
}

void CodeGen::findFunctionData(AstNode::FunctionDeclaration *function, std::ofstream &file) {
    AstNode::Block *block = static_cast<AstNode::Block *>(function->body->data);
    for (AstNode *stmt : block->statements) {
        switch (stmt->type) {
            case AstNodeType::PRINT: {
                AstNode::Print *print = static_cast<AstNode::Print *>(stmt->data);
                if (print->expression->type == AstNodeType::STRING_LITERAL) {
                    AstNode::StringLiteral *string = static_cast<AstNode::StringLiteral *>(print->expression->data);

                    size_t startPos = string->value.find("\"");
                    size_t endPos = string->value.rfind("\"");
                    if (startPos != std::string::npos && endPos != std::string::npos) {
                        string->value = string->value.substr(startPos + 1, endPos - startPos - 1);
                    }

                    file << "   fmt db \"" << string->value << "\", 10\n";
                }
                break;
            }
            case AstNodeType::VAR_DECLARATION: {
                AstNode::VarDeclaration *variable = static_cast<AstNode::VarDeclaration *>(stmt->data);
                generateVariableDeclaration(variable, file);
                break;
            }
        }
    }
}

void CodeGen::findSectionData(AstNode::Program *program, std::ofstream &file) {
    file << "section .data\n";
    for (AstNode *stmt : program->statements) {
        switch (stmt->type) {
            case AstNodeType::FUNCTION_DECLARATION: {
                AstNode::FunctionDeclaration *function = static_cast<AstNode::FunctionDeclaration *>(stmt->data);
                findFunctionData(function, file);
                break;
            }
            case AstNodeType::VAR_DECLARATION: {
                AstNode::VarDeclaration *variable = static_cast<AstNode::VarDeclaration *>(stmt->data);
                generateVariableDeclaration(variable, file);
                break;
            }
        }
    }
    file << "\n";
}

void CodeGen::findSectionText(AstNode::Program *program, std::ofstream &file) {
    file << "section .text\n";
    file << "   extern printf, exit\n";
    file << "\n";
}

void CodeGen::setStack(std::ofstream &file) {
    file << "   ; Set up stack frame\n";
    file << "   push rbp\n";
    file << "   mov rbp, rsp\n\n";
}

void CodeGen::cleanStack(std::ofstream &file) {
    file << "   ; Clean up and return\n";
    file << "   mov rsp, rbp\n";
    file << "   pop rbp\n";
    file << "   ret\n";
}

void CodeGen::createFunction(AstNode::Program *progam, std::ofstream &file) {
    AstNode::FunctionDeclaration *function = static_cast<AstNode::FunctionDeclaration *>(progam->statements[0]->data);
    function->name.start = strtok(const_cast<char*>(function->name.start), "(");
    function->name.start = strtok(const_cast<char*>(function->name.start), " ");
    file << function->name.start << ":\n";

    setStack(file);

    AstNode::Block *block = static_cast<AstNode::Block *>(function->body->data);
    for (AstNode *stmt : block->statements) {
        switch (stmt->type) {
            case AstNodeType::PRINT: {
                file << "   ; Print\n";
                file << "   lea rdi, [fmt]\n";
                file << "   mov rsi, rax\n";
                file << "   xor eax, eax\n";
                file << "   call printf\n\n"; 
                break;
            }
            case AstNodeType::VAR_DECLARATION: {
                AstNode::VarDeclaration *variable = static_cast<AstNode::VarDeclaration *>(stmt->data);
                file << "   ; Variable declaration\n";
                if (variable->initializer->type == AstNodeType::NUMBER_LITERAL) {
                    AstNode::NumberLiteral *number = static_cast<AstNode::NumberLiteral *>(variable->initializer->data);
                    if (number->value <= 127 && number->value >= -128) {
                        file << "   movsx rax, byte[" << variable->name.start << "]\n\n";
                    } else {
                        file << "   movsx rax, word[" << variable->name.start << "]\n\n";
                    }
                }
                break;
            }
            case AstNodeType::EXIT: {
                AstNode::Exit *exit = static_cast<AstNode::Exit *>(stmt->data);
                AstNode::NumberLiteral *number = static_cast<AstNode::NumberLiteral *>(exit->expression->data);
                file << "   ; Exit\n";
                file << "   mov rdi, " << number->value << "\n";
                file << "   xor rax, rax\n";
                file << "   call exit\n\n";
                break;
            }
        }
    }

    cleanStack(file);
}

void AstNode::codeGen(AstNode *expression) {
    std::ofstream file;
    file.open("out.s");

    if (expression->type == AstNodeType::PROGRAM) {
        AstNode::Program *program = static_cast<AstNode::Program *>(expression->data);
        CodeGen::findSectionData(program, file);
        CodeGen::findSectionText(program, file);

        file << "global main\n";
        CodeGen::createFunction(program, file);
    }

    file.close();
}
