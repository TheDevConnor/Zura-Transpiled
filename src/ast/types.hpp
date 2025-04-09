#pragma once

#include "ast.hpp"

#include <iostream>

class SymbolType : public Node::Type {
public:

  enum class Signedness {
    INFER,  // For example, all int literals have this- when you declare an unsigned variable and set it to one of those, it will change
    SIGNED,
    UNSIGNED,
  };
  
  // ex: T, int, float, double, char, etc.
  std::string name;
  // signed int, unsigned int
  Signedness signedness = Signedness::UNSIGNED;


  SymbolType(std::string name, Signedness signedness = Signedness::INFER) : name(name), signedness(signedness) { kind = NodeKind::ND_SYMBOL_TYPE; }

  void debug(int indent = 0) const override { 
    std::cout << "Type: \n";
    Node::printIndent(indent + 1);
    std::cout << name << "\n";
    // signeddness
    Node::printIndent(indent + 1);
    std::cout << "Signedness: ";
    switch (signedness) {
      case Signedness::INFER:
        std::cout << "Inferred\n";
        break;
      case Signedness::SIGNED:
        std::cout << "Signed\n";
        break;
      case Signedness::UNSIGNED:
        std::cout << "Unsigned\n";
        break;
      default:
        std::cout << "Unknown\n";
        break;
    };
  }
};

class ArrayType : public Node::Type {
public:
  Node::Type *underlying; // []int - 'int' is the underlying type
  long long int constSize; // [45]int - 45 is the constant, maximum size

  ArrayType(Node::Type *underlying, long long int constSize) : underlying(underlying), constSize(constSize) {
    kind = NodeKind::ND_ARRAY_TYPE;
  }

  ArrayType(ArrayType *&arr) : underlying(arr->underlying), constSize(arr->constSize) {
    kind = NodeKind::ND_ARRAY_TYPE;
  }

  void debug(int indent = 0) const override {
    std::cout << "[]";
    if (underlying) {
      underlying->debug(indent);
    } else std::cout << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Size: " << (constSize < 1 ? "Any" : std::to_string(constSize)) << "\n";
  }

  ~ArrayType() = default;
};

class PointerType : public Node::Type {
public:
  Node::Type *underlying;   // *int

  PointerType(Node::Type *underlying)
      : underlying(underlying) {
    kind = NodeKind::ND_POINTER_TYPE;
  }

  void debug(int indent = 0) const override {
    std::cout << "*";
    underlying->debug(indent);
  }

  ~PointerType() = default; // rule of threes-
};

class TemplateStructType : public Node::Type {
public:
  Node::Type *name; // Person
  Node::Type *underlying; // < int >

  TemplateStructType(Node::Type *name, Node::Type *underlying)
      : name(name), underlying(underlying) {
    kind = NodeKind::ND_TEMPLATE_STRUCT_TYPE;
  }

  void debug(int indent = 0) const override {
    name->debug(indent);
    std::cout << "<";
    underlying->debug(indent);
    std::cout << ">";
  }

  ~TemplateStructType() = default;
};

class FunctionType : public Node::Type {
public:
  std::vector<Node::Type *> args;
  Node::Type *ret;

  FunctionType(std::vector<Node::Type *> args, Node::Type *ret)
      : args(args), ret(ret) {
    kind = NodeKind::ND_FUNCTION_TYPE;
  }

  void debug(int indent = 0) const override {
    std::cout << "fn (";
    for (auto &arg : args) {
      arg->debug(indent);
      std::cout << ", ";
    }
    std::cout << ") ";
    ret->debug(indent);
  }

  ~FunctionType() = default;
};