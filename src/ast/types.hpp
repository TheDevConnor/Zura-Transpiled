#pragma once

#include "ast.hpp"

#include <iostream>

class SymbolType : public Node::Type {
public:
    // ex: T, int, float, double, char, etc.
    std::string name;

    SymbolType(std::string name) : name(name) {
        kind = NodeKind::ND_SYMBOL_TYPE;
    }

    void debug() const override {
        std::cout << name;
    }
};

class ArrayType : public Node::Type {
public:
    Node::Type *underlying; // []int

    ArrayType(Node::Type *underlying) : underlying(underlying) {
        kind = NodeKind::ND_ARRAY_TYPE;
    }

    void debug() const override {
        std::cout << "[]";
        underlying->debug();
    }
};