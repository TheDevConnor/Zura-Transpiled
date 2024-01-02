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
  struct MinMaxType;

  constexpr static int8_t INT8_min = -128;
  constexpr static int8_t INT8_max = 127;
  constexpr static int16_t INT16_min = -32768;
  constexpr static int16_t INT16_max = 32767;
  constexpr static int32_t INT32_min = -2147483648;
  constexpr static int32_t INT32_max = 2147483647;
  constexpr static double INT64_min = -9.223372036854776e+18;
  constexpr static double INT64_max = 9.223372036854776e+18;
  constexpr static __int128_t INT128_min = -1.7014118346e+38;
  constexpr static __int128_t INT128_max = 1.7014118346e+38;

  constexpr static float FLOAT32_max = 3.402823466e+38F;
  constexpr static float FLOAT32_min = -3.402823466e+38F;
  constexpr static double FLOAT64_max = 1.7976931348623158e+308;
  constexpr static double FLOAT64_min = -1.7976931348623158e+308;

  static std::unordered_map<std::string, AstNode::Type *> typeMap;
  static const std::vector<std::vector<std::string>> upCastArray;
  static std::unordered_map<std::string, int> typeIndices;
  static const std::vector<MinMaxType> typeArray;

  struct MinMaxType {
    double min;
    double max;
    AstNode::Type *type;
  };

  static void checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                             std::string name);

  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);

  static AstNode::Type *determineIfUpCast(AstNode::Type *type,
                                          AstNode::Type *returnType);
  static AstNode::Type *findType(const std::string &typeName);
  static AstNode::Type *determineType(double value);
};
