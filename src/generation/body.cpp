#include <fstream>

#include "../ast/ast.hpp"
#include "gen.hpp"

void Gen::exitStmt(std::ofstream &file, AstNode* node) {
    AstNode::Exit *exit = (AstNode::Exit *)node->data;
    if (exit->expression->type == AstNodeType::NUMBER_LITERAL) {
        AstNode::NumberLiteral *number = (AstNode::NumberLiteral *)exit->expression->data;
        file << "  return " << number->value << ";\n";
    }
}

void Gen::blockStmt(std::ofstream &file, AstNode* node) {
    AstNode::Block *block = (AstNode::Block *)node->data;
    for (AstNode *stmt : block->statements) {
        switch (stmt->type) {
            case AstNodeType::EXIT: exitStmt(file, stmt); break;
            default: break;
        }
    }
}

void Gen::body(std::ofstream &file, AstNode* node) {
  if (ast->type == AstNodeType::PROGRAM) {
    AstNode::Program *program = (AstNode::Program *)ast->data;

    for (AstNode *node : program->statements) {
      switch (node->type) {
        case AstNodeType::FUNCTION_DECLARATION: functionDeclaration(file, node); break;
        case AstNodeType::BLOCK: blockStmt(file, node); break;
        case AstNodeType::EXIT: exitStmt(file, node); break;
        default: break;
      }
    }
  }
}