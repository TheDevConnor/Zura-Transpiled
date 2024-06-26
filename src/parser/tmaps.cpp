#include "../ast/types.hpp"
#include "parser.hpp"

Lexer lex;

template <typename T, typename U>
T Parser::lookup(const std::vector<std::pair<U, T>> &lu, U key) {
    auto it = std::find_if(lu.begin(), lu.end(), [key](auto &p) {
        return p.first == key;
    });

    if (it == lu.end()) {
        std::cerr << "No value found for key " << lex.tokenToString(key) << std::endl;
        throw std::runtime_error("No value found for key");
    }

    return it->second;
}

void Parser::createTypeMaps() {
    type_nud_lu = {
        { TokenKind::IDENTIFIER, symbol_table },
        { TokenKind::LEFT_BRACKET, array_type },
        { TokenKind::STAR, pointer_type }, // * 
        { TokenKind::LAND, pointer_type }, // &
    };
    type_led_lu = {};
    type_bp_lu = bp_lu;
}

Node::Type *Parser::type_led(PStruct *psr, Node::Type *left, BindingPower bp) {
    auto op = psr->current(psr);
    try {
        return lookup(type_led_lu, op.kind)(psr, left, bp);
    } catch (std::exception &e) {
        std::cerr << "Error in type_led: " << e.what() << std::endl;
        return nullptr;
    }
    return nullptr;
}

Node::Type *Parser::type_nud(PStruct *psr) {
    auto op = psr->current(psr);
    try {
        return Parser::lookup(type_nud_lu, op.kind)(psr);
    } catch (std::exception &e) {
        std::cerr << "Error in type_nud: " << e.what() << std::endl;
        return nullptr;
    }
    return nullptr;
}

Parser::BindingPower Parser::type_getBP(TokenKind tk) {
    return lookup(type_bp_lu, tk);
}
