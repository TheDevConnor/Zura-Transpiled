#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "../ast/ast.hpp"
#include "type.hpp"

#define FLOAT32_MAX 3.402823466e+38F
#define FLOAT32_MIN -3.402823466e+38F

std::unordered_map<std::string, AstNode::Type *> Type::findType = {
    {"i8",   new AstNode::Type(Lexer::Token{.start = "i8"})},
    {"i16",  new AstNode::Type(Lexer::Token{.start = "i16"})},
    {"i32",  new AstNode::Type(Lexer::Token{.start = "i32"})},
    {"i64",  new AstNode::Type(Lexer::Token{.start = "i64"})},
    {"i128", new AstNode::Type(Lexer::Token{.start = "i128"})},
    {"f32",  new AstNode::Type(Lexer::Token{.start = "f32"})},
    {"f64",  new AstNode::Type(Lexer::Token{.start = "f64"})},
    {"str",  new AstNode::Type(Lexer::Token{.start = "str"})},
    {"bool", new AstNode::Type(Lexer::Token{.start = "bool"})},
    {"void", new AstNode::Type(Lexer::Token{.start = "void"})}};

AstNode::Type *Type::i8   = findType["i8"];
AstNode::Type *Type::i16  = findType["i16"];
AstNode::Type *Type::i32  = findType["i32"];
AstNode::Type *Type::i64  = findType["i64"];
AstNode::Type *Type::i128 = findType["i128"];
AstNode::Type *Type::f32  = findType["f32"];
AstNode::Type *Type::f64  = findType["f64"];

AstNode::Type *Type::str  = findType["str"];
AstNode::Type *Type::bool_ = findType["bool"];
AstNode::Type *Type::void_ = findType["void"];

void Type::determineType(double value) {
  if (value == (int)value) {
    if (value >= INT8_MIN && value <= INT8_MAX) {
      returnType = i8; return;
    } else if (value >= INT16_MIN && value <= INT16_MAX) {
      returnType = i16; return;
    } else if (value >= INT32_MIN && value <= INT32_MAX) {
      returnType = i32; return;
    } else if (value >= INT64_MIN && value <= INT64_MAX) {
      returnType = i64; return;
    } else {
      returnType = i128; return;
    }
  } else {
    if (value >= FLOAT32_MIN && value <= FLOAT32_MAX) {
      returnType = f32; return;
    } else {
      returnType = f64; return;
    }
  }
}

AstNode::Type *Type::determineIfUpCast(AstNode::Type *type,
                                       AstNode::Type *returnType) {
  if (strcmp(type->type.start, returnType->type.start) == 0)
    return type;

  if (strcmp(type->type.start, "i8") == 0) {
    if (strcmp(returnType->type.start, "i16") == 0 ||
        strcmp(returnType->type.start, "i32") == 0 ||
        strcmp(returnType->type.start, "i64") == 0 ||
        strcmp(returnType->type.start, "i128") == 0 ||
        strcmp(returnType->type.start, "f32") == 0 ||
        strcmp(returnType->type.start, "f64") == 0) {
      type = i16;
    }
  }

  if (strcmp(type->type.start, "i16") == 0) {
    if (strcmp(returnType->type.start, "i32") == 0 ||
        strcmp(returnType->type.start, "i64") == 0 ||
        strcmp(returnType->type.start, "i128") == 0 ||
        strcmp(returnType->type.start, "f32") == 0 ||
        strcmp(returnType->type.start, "f64") == 0) {
      type = i32;
    }
  }

  if (strcmp(type->type.start, "i32") == 0) {
    if (strcmp(returnType->type.start, "i64") == 0 ||
        strcmp(returnType->type.start, "i128") == 0 ||
        strcmp(returnType->type.start, "f32") == 0 ||
        strcmp(returnType->type.start, "f64") == 0) {
      type = i64;
    }
  }

  if (strcmp(type->type.start, "i64") == 0) {
    if (strcmp(returnType->type.start, "i128") == 0 ||
        strcmp(returnType->type.start, "f32") == 0 ||
        strcmp(returnType->type.start, "f64") == 0) {
      type = i128;
    }
  }

  if (strcmp(type->type.start, "i128") == 0) {
    if (strcmp(returnType->type.start, "f32") == 0 ||
        strcmp(returnType->type.start, "f64") == 0) {
      returnType = f32;
    }
  }

  if (strcmp(type->type.start, "f32") == 0) {
    if (strcmp(returnType->type.start, "f64") == 0) {
      type = f64;
    }
  }

  return type;
}
