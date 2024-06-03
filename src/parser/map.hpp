#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <iostream>

#include "../lexer/lexer.hpp"
#include "parser.hpp"

using namespace ParserClass;

const std::unordered_map<TokenKind, BindingPower> ParserClass::bp_table = {
    // Literals
    {TokenKind::NUMBER, BindingPower::defaultValue},
    {TokenKind::IDENTIFIER, BindingPower::defaultValue},
    {TokenKind::STRING, BindingPower::defaultValue},
    {TokenKind::IDENTIFIER, BindingPower::defaultValue},

    // Parentheses
    {TokenKind::LEFT_PAREN, BindingPower::call},

    // Binary operators
    {TokenKind::PLUS, BindingPower::additive},
    {TokenKind::MINUS, BindingPower::additive},

    {TokenKind::STAR, BindingPower::multiplicative},
    {TokenKind::SLASH, BindingPower::multiplicative},
    {TokenKind::MODULO, BindingPower::multiplicative},

    {TokenKind::CARET, BindingPower::power},

    // Comparison
    {TokenKind::LESS, BindingPower::comparison},
    {TokenKind::GREATER, BindingPower::comparison},
    {TokenKind::LESS_EQUAL, BindingPower::comparison},
    {TokenKind::GREATER_EQUAL, BindingPower::comparison},
    {TokenKind::EQUAL_EQUAL, BindingPower::comparison},
    {TokenKind::BANG_EQUAL, BindingPower::comparison},

    // Unary
    {TokenKind::BANG, BindingPower::prefix},
};

const std::unordered_map<TokenKind, nud_t> ParserClass::nud_table = {
    // Literals
    {TokenKind::NUMBER, &num},
    {TokenKind::IDENTIFIER, &ident},
    {TokenKind::STRING, &str},

    // Prefix
    {TokenKind::MINUS, &unary},
    {TokenKind::BANG, &unary},
};

const std::unordered_map<TokenKind, led_t> ParserClass::led_table = {
    // Binary
    {TokenKind::PLUS, &binary},
    {TokenKind::MINUS, &binary},
    {TokenKind::STAR, &binary},
    {TokenKind::SLASH, &binary},
    {TokenKind::MODULO, &binary},
    {TokenKind::CARET, &binary},

    // Comparison
    {TokenKind::LESS, &binary},
    {TokenKind::GREATER, &binary},
    {TokenKind::LESS_EQUAL, &binary},
    {TokenKind::GREATER_EQUAL, &binary},
    {TokenKind::EQUAL_EQUAL, &binary},
    {TokenKind::BANG_EQUAL, &binary},
};

BindingPower ParserClass::getBP(Parser *psr, TokenKind tk) {
    if (ParserClass::bp_table.find(tk) == ParserClass::bp_table.end()) {
        std::cout << "Error: No binding power found for token kind: " << tk << std::endl;
        return BindingPower::err;
    } 
    return ParserClass::bp_table.at(tk);
}

Node::Expr *ParserClass::nudHandler(Parser *psr, TokenKind tk) {
    if (nud_table.find(tk) == nud_table.end()) {
        std::cout << "Error: No nud handler found for token kind: " << tk << std::endl;
        return nullptr;
    }

    nud_t nudFunc = nud_table.at(tk);
    return nudFunc(psr); 
}

Node::Expr *ParserClass::ledHandler(Parser *psr, Node::Expr* left) {
    auto op = psr->current(psr);
    auto bp = ParserClass::getBP(psr, op.kind);

    if (led_table.find(op.kind) == led_table.end()) {
        std::cout << "Error: No led handler found for token kind: " << op.kind << std::endl;
        return nullptr;
    }

    led_t ledFunc = led_table.at(psr->current(psr).kind);
    return ledFunc(psr, left, op.value, bp); 
}