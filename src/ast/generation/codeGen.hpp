#pragma once

#include <fstream>

#include "../ast.hpp"

class CodeGen {
public:
    static void findFunctionData(AstNode::FunctionDeclaration *function, std::ofstream &file);
    static void findSectionData(AstNode::Program *program, std::ofstream &file);
    static void findSectionText(AstNode::Program *program, std::ofstream &file);
    static void findType(AstNode *type, std::ofstream &file);

    static void generateVariableDeclaration(AstNode::VarDeclaration *variable, std::ofstream &file);

    static void createFunction(AstNode::Program *program, std::ofstream &file);
    static void setStack(std::ofstream &file);
    static void cleanStack(std::ofstream &file);
};