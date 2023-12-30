#pragma once

#include "../ast/ast.hpp"

class Type {
public:
  Type(AstNode *expression) : expression(expression) {}
  static void typeCheck(AstNode *expression);

private:
  AstNode *expression;

  static void checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                             std::string name);
  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);
};
