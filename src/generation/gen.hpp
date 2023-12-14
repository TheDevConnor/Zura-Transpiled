#pragma once

#include "../ast/ast.hpp"

class Gen {
public:
  AstNode *ast;
  ~Gen() { delete ast; }

  Gen(AstNode *ast);
  void generate();
};
