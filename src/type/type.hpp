#pragma once

#include "../ast/ast.hpp"
#include <unordered_map>
#include <memory>

inline std::unique_ptr<TypeAST> returnType;
inline std::unique_ptr<TypeAST> type;
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
    std::unique_ptr<TypeAST> type;
  };

  static std::vector<std::pair<std::string, std::unique_ptr<TypeAST>>> paramData;
  static std::unordered_map<std::string, std::unique_ptr<TypeAST>> findType;
  static const std::vector<MinMaxType> typeArray;

  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);

  static void determineType(double value);
  static void resetType();
};
