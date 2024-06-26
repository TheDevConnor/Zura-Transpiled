#include "../ast/types.hpp"
#include "parser.hpp"

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

void Parser::createTypeMaps() {
    type_nud_lu = {
        { TokenKind::IDENTIFIER, symbol_table },
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

Node::Type *Parser::symbol_table(PStruct *psr) {
    return new SymbolType(psr->expect(psr, TokenKind::IDENTIFIER).value);
}

Node::Type *Parser::parseType(PStruct *psr, BindingPower bp) {
    auto left = type_nud(psr);

    // TODO: Fix the weird bug with the getBP function for types
    // while (type_getBP(psr->current(psr).kind) > bp){
    //     left = type_led(psr, left, type_getBP(psr->current(psr).kind));
    // }

    return left;
}