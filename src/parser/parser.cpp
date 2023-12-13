#include <iostream>
#include <vector>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Parser::Parser(const char* source) : source(source) {}

Parser::~Parser() {
    delete[] previousToken.start;
    delete[] currentToken.start;
}

AstNode* Parser::parse() {
    lexer.initToken(source);

    advance();

    std::vector<AstNode*> statements = lookupMain();
    AstNode* main = new AstNode(AstNodeType::PROGRAM, new AstNode::Program(statements));

    return main;
}