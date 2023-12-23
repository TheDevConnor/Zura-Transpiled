#pragma once
#include "../ast/ast.hpp"

class Gen {
public:
  AstNode *ast;

  Gen(AstNode *ast);

  bool globalVar = false;
  bool localVar = false;

  // Helpers
  void printTypeToFile(std::ofstream &file, const char *type);
  const char *findType(AstNode *node);

  // Header
  void findBinaryOp(std::ofstream &file, AstNode::Binary *op);
  void expression(std::ofstream &file, AstNode *node);
  void secData(std::ofstream &file, AstNode::Program *node);
  void body(std::ofstream &file, AstNode *node);
  void headerImport(std::ofstream &file);
  void prologue(std::ofstream &file);
  void epilogue(std::ofstream &file);
  void secText(std::ofstream &file);
  void generate();

  // Body
  void varDeclaration(std::ofstream &file, AstNode *node, int indent);
  void functionDeclaration(std::ofstream &file, AstNode *node);
  void returnStmt(std::ofstream &file, AstNode *node);
  void printStmt(std::ofstream &file, AstNode *node);
  void blockStmt(std::ofstream &file, AstNode *node);
  void exitStmt(std::ofstream &file, AstNode *node);
  void callStmt(std::ofstream &file, AstNode *node);
};
