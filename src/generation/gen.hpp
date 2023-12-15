#pragma once

#include "../ast/ast.hpp"

class Gen {
public:
  AstNode *ast;
  ~Gen() { delete ast; }

  Gen(AstNode *ast);

  const char* findType(AstNode *node);
  void printTypeToFile(std::ofstream &file, const char* type);

  void generate();
  void headerImport(std::ofstream &file);
  void functionDeclaration(std::ofstream &file, AstNode::FunctionDeclaration *functionDeclaration);
};
