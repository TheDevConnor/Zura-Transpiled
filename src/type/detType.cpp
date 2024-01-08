#include <cstdint>
#include <cstring>
#include <unordered_map>

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

std::unordered_map<std::string, AstNode::Type *> Type::findType = {
    {"i8", new AstNode::Type(Lexer::Token{.start = "i8"})},
    {"i16", new AstNode::Type(Lexer::Token{.start = "i16"})},
    {"i32", new AstNode::Type(Lexer::Token{.start = "i32"})},
    {"i64", new AstNode::Type(Lexer::Token{.start = "i64"})},
    {"i128", new AstNode::Type(Lexer::Token{.start = "i128"})},
    {"f32", new AstNode::Type(Lexer::Token{.start = "f32"})},
    {"f64", new AstNode::Type(Lexer::Token{.start = "f64"})},
    {"str", new AstNode::Type(Lexer::Token{.start = "str"})},
    {"bool", new AstNode::Type(Lexer::Token{.start = "bool"})},
    {"void", new AstNode::Type(Lexer::Token{.start = "void"})}};

const std::vector<Type::MinMaxType> Type::typeArray = {
    {INT8_min, INT8_max, findType["i8"]},
    {INT16_min, INT16_max, findType["i16"]},
    {INT32_min, INT32_max, findType["i32"]},
    {INT64_min, INT64_max, findType["i64"]},
    {INT128_min, INT128_max, findType["i128"]},
    {FLOAT32_min, FLOAT32_max, findType["f32"]},
    {FLOAT64_min, FLOAT64_max, findType["f64"]},
    {0, 0, findType["str"]},
    {0, 0, findType["bool"]},
    {0, 0, findType["void"]}};

std::vector<std::pair<std::string, AstNode::Type *>> Type::paramData {};

void Type::determineType(double value) {
  for (const MinMaxType &type : typeArray) {
    if (value >= type.min && value <= type.max) {
      returnType = type.type;
      return;
    }
  }
}

void Type::resetType() {
  returnType = nullptr;
  type = nullptr;
  name = "";
}
