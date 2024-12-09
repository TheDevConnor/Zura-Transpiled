#pragma once

#include "ast.hpp"

#include <iostream>

class SymbolType : public Node::Type {
public:
  // ex: T, int, float, double, char, etc.
  std::string name;

  SymbolType(std::string name) : name(name) { kind = NodeKind::ND_SYMBOL_TYPE; }

  void debug(int indent = 0) const override { 
    std::cout << "Type: \n";
    Node::printIndent(indent + 1);
    std::cout << name << "\n";
  }
};

class ArrayType : public Node::Type {
public:
  Node::Type *underlying; // []int - 'int' is the underlying type
  signed short int constSize; // [45]int - 45 is the constant, maximum size

  ArrayType(Node::Type *underlying, signed short int constSize) : underlying(underlying), constSize(constSize) {
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