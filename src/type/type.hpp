#pragma once

#include "../ast/ast.hpp"
#include <memory>
#include <unordered_map>

inline std::unique_ptr<TypeAST> returnType;
inline std::unique_ptr<TypeAST> type;
inline std::string name;

struct MinMaxType {
  double min;
  double max;
  TypeAST *type;

  MinMaxType(double _min, double _max, TypeAST *_type)
      : min(_min), max(_max), type(_type) {}

  MinMaxType(const MinMaxType &other)
      : min(other.min), max(other.max), type(other.type) {}
};

class TypeClass {
public:
  static void typeCheck(const AstNode &node);
  static void determineType(double value);

private:
  static std::vector<std::pair<std::string, std::unique_ptr<TypeAST>>>
      paramData;
  static std::unordered_map<std::string, TypeAST *> findType;
  static const std::vector<MinMaxType> typeArray;
};
