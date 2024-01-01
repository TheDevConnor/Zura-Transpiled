#pragma once
#include "../ast/ast.hpp"

class Gen {
public:
  AstNode *ast;
  Gen(AstNode *ast);

  void generate();
};
