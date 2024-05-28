#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Lexer lexer;
Parser psr;

void ParserClass::storeToken(Parser *psr, Lexer::Token tk) {
    lexer.scanToken();
    while(tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        lexer.scanToken();
    }
}

Node::Stmt ParserClass::parse() {
   storeToken(&psr, lexer.scanToken());

   auto expr = ParserClass::parseExpr(&psr, BindingPower::defaultValue);

   return Node::Stmt();
}