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
		{ TokenKind::CONST, constStmt },
		{ TokenKind::VAR, varStmt },
		{ TokenKind::LEFT_BRACE, blockStmt },
		{ TokenKind::FUN, funStmt },
		{ TokenKind::RETURN, returnStmt },
		{ TokenKind::IF, ifStmt },
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

		{ TokenKind::EQUAL, assign},
		{ TokenKind::PLUS_EQUAL, assign },
		{ TokenKind::MINUS_EQUAL, assign },
		{ TokenKind::STAR_EQUAL, assign },
		{ TokenKind::SLASH_EQUAL, assign },

		{ TokenKind::QUESTION, _ternary },
		{ TokenKind::COLON, _ternary },

		{ TokenKind::LEFT_PAREN, parse_call },

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

		{ TokenKind::EQUAL, BindingPower::assignment},
		{ TokenKind::PLUS_EQUAL, BindingPower::assignment },
		{ TokenKind::MINUS_EQUAL, BindingPower::assignment },
		{ TokenKind::STAR_EQUAL, BindingPower::assignment },
		{ TokenKind::SLASH_EQUAL, BindingPower::assignment },

		{ TokenKind::QUESTION, BindingPower::ternary },

		{ TokenKind::IDENTIFIER, BindingPower::defaultValue },
		{ TokenKind::NUMBER, BindingPower::defaultValue },
		{ TokenKind::STRING, BindingPower::defaultValue },

		// TODO: Make it so that i do not need to add these 
		{ TokenKind::SEMICOLON, BindingPower::defaultValue },
		{ TokenKind::COLON, BindingPower::defaultValue },
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

Node::Stmt *Parser::stmt(PStruct *psr, std::string name) {
	auto stmt_it = std::find_if(stmt_lu.begin(), stmt_lu.end(), [psr](auto &p) {
		return p.first == psr->current(psr).kind;
	}); 

	return stmt_it != stmt_lu.end() ? stmt_it->second(psr, name) : nullptr;
}

BindingPower Parser::getBP(TokenKind tk) {
	return lookup(bp_lu, tk);
}