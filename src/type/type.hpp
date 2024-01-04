#pragma once

#include "../ast/ast.hpp"
#include <unordered_map>

inline AstNode::Type *returnType;
inline AstNode::Type *type;
inline std::string name;

class Type {
public:
  Type(AstNode *expression) : expression(expression) {}
  static void typeCheck(AstNode *expression);

private:
  AstNode *expression;

  struct MinMaxType {
    double min;
    double max;
    AstNode::Type *type;
  };

  static std::unordered_map<std::string, AstNode::Type *> findType;
  static const std::vector<MinMaxType> typeArray;

  static void checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                             std::string name);

  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);

  static void determineType(double value);
};
