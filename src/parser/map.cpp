#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <iostream>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

using namespace Parser;

template <typename T, typename U>
T Parser::lookup(const std::vector<std::pair<U, T>> &lu, U key) {
	auto it = std::find_if(lu.begin(), lu.end(), [key](auto &p) {
		return p.first == key;
	});

	if (it == lu.end()) {
		std::cerr << "No value found for key " << key << std::endl;
		throw std::runtime_error("No value found for key");
	}

	return it->second;
}

void Parser::createMaps() {
	stmt_lu = {
		{ TokenKind::VAR, varStmt },
	};
	nud_lu = {
		{ TokenKind::NUMBER, primary },
		{ TokenKind::IDENTIFIER, primary },
		{ TokenKind::STRING, primary },
		{ TokenKind::LEFT_PAREN, group },
		{ TokenKind::MINUS, unary },
	};
	led_lu = {
		{ TokenKind::PLUS, binary },
		{ TokenKind::MINUS, binary },

		{ TokenKind::STAR, binary },
		{ TokenKind::SLASH, binary },
		{ TokenKind::CARET, binary },
		{ TokenKind::MODULO, binary },

		{ TokenKind::EQUAL_EQUAL, binary },
		{ TokenKind::BANG_EQUAL, binary },
		{ TokenKind::GREATER, binary },
		{ TokenKind::GREATER_EQUAL, binary },
		{ TokenKind::LESS, binary },
		{ TokenKind::LESS_EQUAL, binary },

		{ TokenKind::AND, binary },
		{ TokenKind::OR, binary },
	};
	bp_lu = {
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
		{ TokenKind::RIGHT_PAREN, BindingPower::defaultValue },
	};
}

Node::Expr *Parser::nud(PStruct *psr) {
	auto op = psr->current(psr);
	try {
		return lookup(nud_lu, op.kind)(psr);
	} catch (std::exception &e) {
		std::cerr << "Error in nud: " << e.what() << std::endl;
		return nullptr;
	}
}

Node::Expr *Parser::led(PStruct *psr, Node::Expr *left, BindingPower bp) {
	auto op = psr->current(psr);
	try {
		return lookup(led_lu, op.kind)(psr, left, bp);
	} catch (std::exception &e) {
		std::cerr << "Error in led: " << e.what() << std::endl;
		return nullptr;
	}
}

Node::Stmt *Parser::stmt(PStruct *psr) {
	auto stmt_it = lookup(stmt_lu, psr->current(psr).kind);

	return stmt_it(psr);
}

BindingPower Parser::getBP(TokenKind tk) {
	return lookup(bp_lu, tk);
}