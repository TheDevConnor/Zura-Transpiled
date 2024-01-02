#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "type.hpp"

std::vector<std::pair<std::string, std::string>> Type::paramTypes = {};
// std::vector<std::pair<std::string, std::string>> Type::varTypes   = {};

std::unordered_map<std::string, AstNode::Type *> Type::typeMap = {
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
    {INT8_min, INT8_max, typeMap["i8"]},
    {INT16_min, INT16_max, typeMap["i16"]},
    {INT32_min, INT32_max, typeMap["i32"]},
    {INT64_min, INT64_max, typeMap["i64"]},
    {INT128_min, INT128_max, typeMap["i128"]},
    {FLOAT32_min, FLOAT32_max, typeMap["f32"]},
    {FLOAT64_min, FLOAT64_max, typeMap["f64"]},
    {0, 0, typeMap["str"]}, {0, 0, typeMap["bool"]}, 
    {0, 0, typeMap["void"]}};

const std::vector<std::vector<std::string>> Type::upCastArray = {
    {"i8", "i16", "i32", "i64", "i128", "f32", "f64"}, // i8
    {"i16", "i32", "i64", "i128", "f32", "f64"},       // i16
    {"i32", "i64", "i128", "f32", "f64"},              // i32
    {"i64", "i128", "f32", "f64"},                     // i64
    {"i128", "f32", "f64"},                            // i128
    {"f32", "f64"},                                    // f32
    {"f64"}};                                          // f64 (no upcast)

const std::vector<std::vector<std::string>> Type::downCastArray = {
    {"i8"},                         // i8
    {"i8", "i16"},                  // i16
    {"i8", "i16", "i32"},           // i32
    {"i8", "i16", "i32", "i64"},    // i64
    {"i8", "i16", "i32", "i64"},    // i128
    {"i8", "i16", "i32", "i64"},    // f32
    {"i8", "i16", "i32", "i64"}};   // f64

std::unordered_map<std::string, int> Type::typeIndices = {
    {"i8", 0}, {"i16", 1}, {"i32", 2}, {"i64", 3}, 
    {"i128", 4}, {"f32", 5}, {"f64", 6}, {"str", 7}};

AstNode::Type *Type::findType(const std::string &typeName) {
  auto it = typeMap.find(typeName);
  return (it != typeMap.end()) ? it->second : nullptr;
}

AstNode::Type *Type::determineType(double value) {
  for (auto &type : typeArray) {
    if (value >= type.min && value <= type.max) {
      return type.type;
    }
  }
  return nullptr;
}

AstNode::Type *Type::determineIfUpCast(AstNode::Type *type,
                                       AstNode::Type *returnType) {
  if (type == returnType)
    return type;

  int typeIndex = typeIndices[type->type.start];

  auto it = std::find(upCastArray[typeIndex].begin(),
                      upCastArray[typeIndex].end(), returnType->type.start);

  if (it != upCastArray[typeIndex].end())
    return returnType;

  return type;
}

AstNode::Type *Type::determineIfDownCast(AstNode::Type *type,
                                         AstNode::Type *returnType) {
  if (type == returnType)
    return type;

  int typeIndex = typeIndices[type->type.start];

  auto it = std::find(downCastArray[typeIndex].begin(),
                      downCastArray[typeIndex].end(), returnType->type.start);

 if (it != downCastArray[typeIndex].end())
    return returnType;

  return type;
}