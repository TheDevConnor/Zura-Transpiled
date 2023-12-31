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

  static std::vector<std::pair<std::string, AstNode::Type*>> paramData;
  static std::unordered_map<std::string, AstNode::Type *> findType;
  static const std::vector<MinMaxType> typeArray;

  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);

  static void determineType(double value);
  static void resetType();
};
