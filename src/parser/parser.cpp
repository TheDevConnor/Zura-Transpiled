#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Parser::Parser(const char* source) : source(source) {}

AstNode* Parser::parse() {
    lexer.initToken(source);

    advance();

    AstNode* expr = expression();
    consume(TokenKind::END_OF_FILE, "Expected end of expression");

    expr->printAst(expr, 0);

    return expr;
}