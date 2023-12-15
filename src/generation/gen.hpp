#pragma once

#include "../ast/ast.hpp"

class Gen {
public:
  AstNode *ast;
  ~Gen() { delete ast; }

  Gen(AstNode *ast);

  // Helpers
  const char* findType(AstNode *node);
  void printTypeToFile(std::ofstream &file, const char* type);

  // Header
  void generate();
  void headerImport(std::ofstream &file);
  void body(std::ofstream &file, AstNode* node);

  // Body
  // void varDeclaration(std::ofstream &file, AstNode* node);
  void functionDeclaration(std::ofstream &file, AstNode* node);
  void exitStmt(std::ofstream &file, AstNode* node);
  void blockStmt(std::ofstream &file, AstNode* node);
};
