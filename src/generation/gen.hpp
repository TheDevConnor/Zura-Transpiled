#pragma once

#include "../ast/ast.hpp"
#include <memory>

class Gen {
public:
  std::unique_ptr<AstNode> ast;

  Gen(std::unique_ptr<AstNode> ast);

  // Helpers
  void printTypeToFile(std::ofstream &file, const char *type);
  const char *findType(AstNode *node);

  // Header
  void findBinaryOp(std::ofstream &file, AstNode::Binary *op);
  void expression(std::ofstream &file, std::unique_ptr<AstNode> node);
  void body(std::ofstream &file, AstNode *node);
  void headerImport(std::ofstream &file);
  void generate();

  // Body
  void varDeclaration(std::ofstream &file, std::unique_ptr<AstNode> node,
                      int indent);
  void functionDeclaration(std::ofstream &file, std::unique_ptr<AstNode> node);
  void blockStmt(std::ofstream &file, AstNode *node);
  void exitStmt(std::ofstream &file, std::unique_ptr<AstNode> node);
};
