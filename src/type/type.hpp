#pragma once

#include <unordered_map>
#include "../ast/ast.hpp"


inline AstNode::Type *returnType;
inline AstNode::Type *type;
inline std::string name;

class Type {
public:
  Type(AstNode *expression) : expression(expression) {}
  static void typeCheck(AstNode *expression);

private:
  AstNode *expression;

  static AstNode::Type *i8;  
  static AstNode::Type *i16;  
  static AstNode::Type *i32;  
  static AstNode::Type *i64;  
  static AstNode::Type *i128; 
  static AstNode::Type *f32;  
  static AstNode::Type *f64;  

  static AstNode::Type *str;
  static AstNode::Type *bool_;
  static AstNode::Type *void_;

  static std::unordered_map<std::string, AstNode::Type *> findType;

  static void checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                             std::string name);
                             
  static void checkExpression(AstNode *expr);
  static void checkBody(AstNode *body);

  static AstNode::Type *determineIfUpCast(AstNode::Type *type, AstNode::Type *returnType);
  static void determineType(double value);
};
