#include "gen.hpp"
#include <cstring>
#include <fstream>

void Gen::secData(std::ofstream &file, AstNode::Program *node) {
  file << "section .data\n";

  for (AstNode *child : node->statements) {
    if (child->type == AstNodeType::VAR_DECLARATION) {
      globalVar = true;
      // (x db 10)
      AstNode::VarDeclaration *var =
          static_cast<AstNode::VarDeclaration *>(child->data);
      var->name.start = strtok(const_cast<char *>(var->name.start), ":");
      file << " ; This is a global var\n";
      file << "\t" << var->name.start << " db ";

      expression(file, var->initializer);
      file << "\n";
    }
    if (child->type == AstNodeType::FUNCTION_DECLARATION) {
      AstNode::FunctionDeclaration *fun =
          static_cast<AstNode::FunctionDeclaration *>(child->data);
      AstNode::Block *block = static_cast<AstNode::Block *>(fun->body->data);

      for (AstNode *stmt : block->statements) {
        if (stmt->type == AstNodeType::PRINT) {
          AstNode::Print *print = static_cast<AstNode::Print *>(stmt->data);
          AstNode::StringLiteral *str =
              static_cast<AstNode::StringLiteral *>(print->expression->data);
          str->value = strtok(const_cast<char *>(str->value.c_str()), "\"");
          file << "\t"
               << "fmt db \"" << str->value << "\", 10, 0\n";
        }
      }
    }
  }

  file << "\n";
}

void Gen::secText(std::ofstream &file) {
  file << "section .text\n";
  file << "\tglobal main\n";
  file << "\textern printf, exit\n";
}

void Gen::headerImport(std::ofstream &file) {
  // section .text
  secText(file);
}
