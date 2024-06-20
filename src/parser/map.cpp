#include <unordered_map>
#include <functional>
#include <string>
#include <iostream>

#include "../lexer/lexer.hpp"
#include "parser.hpp"

using namespace Parser;

Node::Expr * Parser::led(PStruct *psr, Node::Expr *left, BindingPower bp) {
	std::unordered_map<TokenKind, LedHandler> led_lu = {
		// additive 
		{ TokenKind::PLUS, binary },
		{ TokenKind::MINUS, binary },

		// multiplicative
		{ TokenKind::STAR, binary },
		{ TokenKind::SLASH, binary },

		// power && modulo
		{ TokenKind::CARET, binary },
		{ TokenKind::MODULO, binary },

		// comparison
		{ TokenKind::EQUAL_EQUAL, binary },
		{ TokenKind::BANG_EQUAL, binary },
		{ TokenKind::GREATER, binary },
		{ TokenKind::GREATER_EQUAL, binary },
		{ TokenKind::LESS, binary },
		{ TokenKind::LESS_EQUAL, binary },

		// logical
		{ TokenKind::AND, binary },
		{ TokenKind::OR, binary }
	};
	auto op = psr->current(psr);
	auto led_fn = led_lu.find(op.kind);

	if (led_fn == led_lu.end()) {
		std::cerr << "No led function found for " << op.kind << std::endl;
		return nullptr;
	}

	return led_fn->second(psr, left, bp);
}

BindingPower Parser::getBP(TokenKind tk) {
	std::unordered_map<TokenKind, BindingPower> bp_lu = {
		{ TokenKind::COMMA, BindingPower::comma },
		{ TokenKind::EQUAL, BindingPower::assignment },

		{ TokenKind::OR, BindingPower::logicalOr },
		{ TokenKind::AND, BindingPower::logicalAnd },

		{ TokenKind::EQUAL_EQUAL, BindingPower::comparison },
		{ TokenKind::BANG_EQUAL, BindingPower::comparison },
		{ TokenKind::GREATER, BindingPower::comparison },
		{ TokenKind::GREATER_EQUAL, BindingPower::comparison },
		{ TokenKind::LESS, BindingPower::comparison },
		{ TokenKind::LESS_EQUAL, BindingPower::comparison },

		{ TokenKind::PLUS, BindingPower::additive },
		{ TokenKind::MINUS, BindingPower::additive },

		{ TokenKind::STAR, BindingPower::multiplicative },
		{ TokenKind::SLASH, BindingPower::multiplicative },
		{ TokenKind::MODULO, BindingPower::multiplicative },

		{ TokenKind::CARET, BindingPower::power },

		{ TokenKind::LEFT_PAREN, BindingPower::call },

		{ TokenKind::IDENTIFIER, BindingPower::defaultValue },
		{ TokenKind::NUMBER, BindingPower::defaultValue },
		{ TokenKind::STRING, BindingPower::defaultValue },
		{ TokenKind::SEMICOLON, BindingPower::defaultValue },
	};
	auto bp = bp_lu.find(tk);

	if (bp == bp_lu.end()) {
		std::cerr << "No binding power found for " << tk << std::endl;
		return BindingPower::defaultValue;
	}

	return bp->second;
}

Node::Expr * Parser::nud(PStruct *psr) {
	std::unordered_map<TokenKind, NudHandler> nud_lu = {
		{ TokenKind::NUMBER, primary },
		{ TokenKind::IDENTIFIER, primary },
		{ TokenKind::STRING, primary },
		{ TokenKind::LEFT_PAREN, group },
		{ TokenKind::MINUS, unary },
	};
	auto op = psr->current(psr);
	auto nud_fn = nud_lu.find(op.kind);

	if (nud_fn == nud_lu.end()) {
		std::cerr << "No nud function found for " << op.kind << std::endl;
		return nullptr;
	}

	return nud_fn->second(psr);
}