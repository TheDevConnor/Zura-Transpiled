#include "parser.hpp"

void Parser::createTypeMaps() {
    type_nud_lu = {};
    type_led_lu = {};
    type_bp_lu = {};
}

Node::Type *Parser::type_led(PStruct *psr, Node::Expr *left, BindingPower bp) {
    return nullptr;
}

Node::Type *Parser::type_nud(PStruct *psr) {
    return nullptr;
}

Parser::BindingPower Parser::type_getBP(TokenKind tk) {
    return BindingPower::defaultValue;
}