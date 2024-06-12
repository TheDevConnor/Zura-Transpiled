#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <iostream>

#include "../lexer/lexer.hpp"
#include "parser.hpp"

using namespace ParserNamespace;

void ParserNamespace::led(TokenKind kind, BindingPower bp, LedHandler led_fn) {
    bp_lu[kind] = bp;
    led_lu[kind] = led_fn;
}

void ParserNamespace::nud(TokenKind kind, NudHandler nud_fn) {
    nud_lu[kind] = nud_fn;
}

void ParserNamespace::stmt(TokenKind kind, StmtHandler stmt_fn) {
    bp_lu[kind] = BindingPower::defaultValue;
    stmt_lu[kind] = stmt_fn;
}

void ParserNamespace::createTokenLookup() {
    // Logical
	led(TokenKind::AND, logicalAnd, binary);

	// Relational
	led(TokenKind::LESS, relational, binary);
	led(TokenKind::LESS_EQUAL, relational, binary);
	led(TokenKind::GREATER, relational, binary);
	led(TokenKind::GREATER_EQUAL, relational, binary);
	led(TokenKind::EQUAL_EQUAL, relational, binary);
	led(TokenKind::BANG, relational, binary);
    led(TokenKind::BANG_EQUAL, relational, binary);

	// Additive & Multiplicative
	led(TokenKind::PLUS, additive, binary);
	led(TokenKind::MINUS, additive, binary);

	led(TokenKind::STAR, multiplicative, binary);
	led(TokenKind::SLASH, multiplicative, binary);
	led(TokenKind::MODULO, multiplicative, binary);

	// Literals & Symbols
	nud(TokenKind::NUMBER, primary);
	nud(TokenKind::STRING, primary);
	nud(TokenKind::IDENTIFIER, primary);
	nud(TokenKind::LEFT_PAREN, group);
	nud(TokenKind::MINUS, unary);
}