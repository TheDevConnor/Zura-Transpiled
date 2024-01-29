#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/ast.hpp"
#include "type.hpp"

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

// std::unordered_map<std::string, std::unique_ptr<TypeAST>> findType;
std::unordered_map<std::string, TypeAST *> TypeClass::findType;

const std::vector<MinMaxType> TypeClass::typeArray = [] {
  std::unordered_map<std::string, TypeAST *> tempFindType = {
      {"i8", nullptr},   {"i16", nullptr}, {"i32", nullptr}, {"i64", nullptr},
      {"i128", nullptr}, {"f32", nullptr}, {"f64", nullptr}, {"str", nullptr},
      {"bool", nullptr}, {"void", nullptr}};

  // Update the pointers in typeArray
  for (auto &entry : tempFindType) {
    tempFindType[entry.first] = new TypeAST(entry.first);
  }

  const std::vector<MinMaxType> tempTypeArray = {
      {INT8_min, INT8_max, tempFindType["i8"]},
      {INT16_min, INT16_max, tempFindType["i16"]},
      {INT32_min, INT32_max, tempFindType["i32"]},
      {INT64_min, INT64_max, tempFindType["i64"]},
      {INT128_min, INT128_max, tempFindType["i128"]},
      {FLOAT32_min, FLOAT32_max, tempFindType["f32"]},
      {FLOAT64_min, FLOAT64_max, tempFindType["f64"]},
      {0, 0, tempFindType["str"]},
      {0, 0, tempFindType["bool"]},
      {0, 0, tempFindType["void"]},
  };

  TypeClass::findType = tempFindType;
  return tempTypeArray;
}();

std::vector<std::pair<std::string, std::unique_ptr<TypeAST>>>
    TypeClass::paramData{};

void TypeClass::determineType(double value) {
  for (const MinMaxType &type : typeArray) {
    if (value >= type.min && value <= type.max) {
      returnType = std::make_unique<TypeAST>(type.type->Name);
      return;
    }
  }
}
