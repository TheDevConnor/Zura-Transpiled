#include "../ast/types.hpp"
#include "parser.hpp"

#include <iostream>

Node::Type *Parser::parseType(PStruct *psr, BindingPower bp) {
    auto left = type_nud(psr);

    // TODO: Fix the weird bug with the getBP function for types
    // while (type_getBP(psr->current(psr).kind) > bp){
    //     left = type_led(psr, left, type_getBP(psr->current(psr).kind));
    // }    

    return left;
}

Node::Type *Parser::symbol_table(PStruct *psr) {
    return new SymbolType(psr->expect(psr, TokenKind::IDENTIFIER).value);
}

Node::Type *Parser::array_type(PStruct *psr) {
    psr->advance(psr);
    psr->expect(psr, TokenKind::RIGHT_BRACKET);
    auto underlying = parseType(psr, defaultValue);
    return new ArrayType(underlying);
}

Node::Type *Parser::pointer_type(PStruct *psr) {
    auto op = psr->advance(psr);
    auto underlying = parseType(psr, defaultValue);
    return new PointerType(op.value, underlying);
}