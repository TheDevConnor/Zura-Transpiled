#pragma once

#include "ast.hpp"

#include <iostream>

class SymbolType : public Node::Type {
public:
  // ex: T, int, float, double, char, etc.
  std::string name;

  SymbolType(std::string name) : name(name) { kind = NodeKind::ND_SYMBOL_TYPE; }

  void debug(int ident = 0) const override { 
    std::cout << "Name: ";
    Node::printIndent(ident + 1);
    std::cout << name;
  }
};

class ArrayType : public Node::Type {
public:
  Node::Type *underlying; // []int - 'int' is the underlying type

  ArrayType(Node::Type *underlying) : underlying(underlying) {
    kind = NodeKind::ND_ARRAY_TYPE;
  }

  void debug(int ident = 0) const override {
    std::cout << "[]";
    underlying->debug(ident + 1);
  }

  ~ArrayType() { delete underlying; }
};

class PointerType : public Node::Type {
public:
  std::string pointer_type; // * or &
  Node::Type *underlying;   // *int

  PointerType(std::string pointer_type, Node::Type *underlying)
      : pointer_type(pointer_type), underlying(underlying) {
    kind = NodeKind::ND_POINTER_TYPE;
  }

  void debug(int ident = 0) const override {
    std::cout << pointer_type;
    underlying->debug(ident + 1);
  }

  ~PointerType() { delete underlying; }
};